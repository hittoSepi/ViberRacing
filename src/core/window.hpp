#pragma once

#include "types.hpp"
#include <string>
#include <functional>

struct GLFWwindow;

namespace viber {

struct WindowConfig {
    int width = 1280;
    int height = 720;
    std::string title = "ViberRacing";
    bool fullscreen = false;
    bool vsync = true;
    int samples = 4;
};

class Window {
public:
    explicit Window(const WindowConfig& config);
    ~Window();
    
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    
    void pollEvents();
    void swapBuffers();
    
    bool shouldClose() const;
    void setShouldClose(bool close);
    
    vec2 getSize() const;
    void setSize(int width, int height);
    
    float getAspectRatio() const;
    
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    vec2 getMousePosition() const;
    vec2 getMouseDelta() const;
    
    void setResizeCallback(std::function<void(int, int)> callback);
    void setKeyCallback(std::function<void(int, int, int, int)> callback);
    void setMouseCallback(std::function<void(double, double)> callback);
    void setMouseButtonCallback(std::function<void(int, int, int)> callback);
    
    GLFWwindow* getHandle() const { return m_window; }
    
private:
    GLFWwindow* m_window = nullptr;
    WindowConfig m_config;
    
    std::function<void(int, int)> m_resizeCallback;
    std::function<void(int, int, int, int)> m_keyCallback;
    std::function<void(double, double)> m_mouseCallback;
    std::function<void(int, int, int)> m_mouseButtonCallback;
    
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    double m_mouseDeltaX = 0.0;
    double m_mouseDeltaY = 0.0;
    
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};

}
