#if defined(__linux__)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include "window.hpp"
#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

#if defined(__linux__)
#include <GLFW/glfw3native.h>
#include <wayland-client.h>
#endif

namespace viber {

Window::Window(const WindowConfig& config) : m_config(config) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    

    // Disable OpenGL context creation - bgfx will create its own
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SAMPLES, config.samples);
    // Ensure window is visible by default (important for Wayland)
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    
    GLFWmonitor* monitor = nullptr;
    if (config.fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        m_config.width = mode->width;
        m_config.height = mode->height;
    }
    
    m_window = glfwCreateWindow(
        m_config.width,
        m_config.height,
        m_config.title.c_str(),
        monitor,
        nullptr
    );
    
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    // Note: No OpenGL context is created here - bgfx handles rendering
    
    // Ensure window is visible (especially important for Wayland)
    glfwShowWindow(m_window);
    glfwFocusWindow(m_window);
    
    // Process events to let Wayland compositor configure the window
    // This is critical for the surface to become mapped (visible)
    for (int i = 0; i < 10; i++) {
        glfwPollEvents();
    }
    
#if defined(__linux__)
    // On Wayland, force a surface commit to ensure the window becomes visible
    if (isWayland()) {
        struct wl_display* display = (struct wl_display*)glfwGetWaylandDisplay();
        struct wl_surface* surface = (struct wl_surface*)glfwGetWaylandWindow(m_window);
        if (surface && display) {
            wl_surface_commit(surface);
            wl_display_roundtrip(display);  // Wait for compositor to process
            wl_display_flush(display);
            spdlog::debug("Wayland surface committed and roundtrip done");
        }
    }
#endif
    
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    
    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    m_lastMouseX = xpos;
    m_lastMouseY = ypos;
    
    spdlog::info("Window created: {}x{}, fullscreen: {}", 
        m_config.width, m_config.height, config.fullscreen);
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
    spdlog::info("Window destroyed");
}

#if defined(__linux__)
void* Window::getNativeDisplay() const {
    // Try X11 first (for XWayland compatibility), then Wayland
    void* display = glfwGetX11Display();
    if (display) {
        spdlog::debug("getNativeDisplay: Using X11 display = {}", display);
        return display;
    }
    // Fall back to Wayland
    display = glfwGetWaylandDisplay();
    spdlog::debug("getNativeDisplay: Using Wayland display = {}", display);
    return display;
}

void* Window::getNativeWindow() const {
    if (!m_window) {
        spdlog::error("m_window is null!");
        return nullptr;
    }
    // Try X11 first (for XWayland compatibility), then Wayland
    ::Window x11Window = glfwGetX11Window(m_window);
    if (x11Window) {
        spdlog::debug("getNativeWindow: Using X11 window = {}", x11Window);
        return reinterpret_cast<void*>(x11Window);
    }
    // Fall back to Wayland
    void* waylandWindow = glfwGetWaylandWindow(m_window);
    spdlog::debug("getNativeWindow: Using Wayland window = {}", waylandWindow);
    return waylandWindow;
}

bool Window::isWayland() const {
    return glfwGetWaylandDisplay() != nullptr;
}
#endif

void Window::pollEvents() {
    m_mouseDeltaX = 0.0;
    m_mouseDeltaY = 0.0;
    glfwPollEvents();
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::setShouldClose(bool close) {
    glfwSetWindowShouldClose(m_window, close);
}

vec2 Window::getSize() const {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    return vec2(static_cast<float>(width), static_cast<float>(height));
}

void Window::setSize(int width, int height) {
    glfwSetWindowSize(m_window, width, height);
    m_config.width = width;
    m_config.height = height;
}

float Window::getAspectRatio() const {
    auto size = getSize();
    return size.x / size.y;
}

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

vec2 Window::getMousePosition() const {
    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    return vec2(static_cast<float>(xpos), static_cast<float>(ypos));
}

vec2 Window::getMouseDelta() const {
    return vec2(static_cast<float>(m_mouseDeltaX), static_cast<float>(m_mouseDeltaY));
}

void Window::setResizeCallback(std::function<void(int, int)> callback) {
    m_resizeCallback = std::move(callback);
}

void Window::setKeyCallback(std::function<void(int, int, int, int)> callback) {
    m_keyCallback = std::move(callback);
}

void Window::setMouseCallback(std::function<void(double, double)> callback) {
    m_mouseCallback = std::move(callback);
}

void Window::setMouseButtonCallback(std::function<void(int, int, int)> callback) {
    m_mouseButtonCallback = std::move(callback);
}

void Window::setScrollCallback(std::function<void(double, double)> callback) {
    m_scrollCallback = std::move(callback);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->m_config.width = width;
        self->m_config.height = height;
        if (self->m_resizeCallback) {
            self->m_resizeCallback(width, height);
        }
    }
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_keyCallback) {
        self->m_keyCallback(key, scancode, action, mods);
    }
}

void Window::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->m_mouseDeltaX = xpos - self->m_lastMouseX;
        self->m_mouseDeltaY = ypos - self->m_lastMouseY;
        self->m_lastMouseX = xpos;
        self->m_lastMouseY = ypos;
        
        if (self->m_mouseCallback) {
            self->m_mouseCallback(xpos, ypos);
        }
    }
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_mouseButtonCallback) {
        self->m_mouseButtonCallback(button, action, mods);
    }
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_scrollCallback) {
        self->m_scrollCallback(xoffset, yoffset);
    }
}

}
