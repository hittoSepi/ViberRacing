#include "window.hpp"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace viber {

Window::Window(const WindowConfig& config) : m_config(config) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, config.samples);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    
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
    
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(config.vsync ? 1 : 0);
    
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    
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

}
