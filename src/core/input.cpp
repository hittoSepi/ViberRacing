#include "input.hpp"

namespace viber {

void Input::update() {
    m_keysPrevious = m_keys;
    m_mouseButtonsPrevious = m_mouseButtons;
    m_mouseDelta = vec2(0.0f);
    m_mouseScroll = 0.0f;
    
    for (int i = 0; i < 4; ++i) {
        updateGamepad(i);
    }
}

void Input::setKeyState(int key, bool pressed) {
    if (key >= 0 && key < 512) {
        m_keys.set(key, pressed);
    }
}

void Input::setMouseButtonState(int button, bool pressed) {
    if (button >= 0 && button < 8) {
        m_mouseButtons.set(button, pressed);
    }
}

void Input::setMousePosition(float x, float y) {
    m_mousePosition = vec2(x, y);
}

void Input::setMouseDelta(float dx, float dy) {
    m_mouseDelta = vec2(dx, dy);
}

bool Input::isKeyPressed(KeyCode key) const {
    const int k = static_cast<int>(key);
    return k >= 0 && k < 512 && m_keys.test(k);
}

bool Input::isKeyJustPressed(KeyCode key) const {
    const int k = static_cast<int>(key);
    return k >= 0 && k < 512 && m_keys.test(k) && !m_keysPrevious.test(k);
}

bool Input::isKeyJustReleased(KeyCode key) const {
    const int k = static_cast<int>(key);
    return k >= 0 && k < 512 && !m_keys.test(k) && m_keysPrevious.test(k);
}

bool Input::isMouseButtonPressed(MouseButton button) const {
    const int b = static_cast<int>(button);
    return b >= 0 && b < 8 && m_mouseButtons.test(b);
}

bool Input::isMouseButtonJustPressed(MouseButton button) const {
    const int b = static_cast<int>(button);
    return b >= 0 && b < 8 && m_mouseButtons.test(b) && !m_mouseButtonsPrevious.test(b);
}

bool Input::isMouseButtonJustReleased(MouseButton button) const {
    const int b = static_cast<int>(button);
    return b >= 0 && b < 8 && !m_mouseButtons.test(b) && m_mouseButtonsPrevious.test(b);
}

const GamepadState& Input::getGamepad(int index) const {
    static GamepadState empty;
    if (index >= 0 && index < 4) {
        return m_gamepads[index];
    }
    return empty;
}

void Input::updateGamepad(int index) {
    // Gamepad update will be implemented with GLFW joystick API
}

void Input::setAction(const std::string& name, KeyCode key) {
    m_actions[name] = [this, key]() { return isKeyPressed(key); };
}

void Input::setAction(const std::string& name, MouseButton button) {
    m_actions[name] = [this, button]() { return isMouseButtonPressed(button); };
}

void Input::setAction(const std::string& name, GamepadButton button, int gamepadIndex) {
    m_actions[name] = [this, button, gamepadIndex]() {
        const auto& gp = getGamepad(gamepadIndex);
        return gp.connected && gp.buttons.test(static_cast<size_t>(button));
    };
}

bool Input::isActionPressed(const std::string& name) const {
    auto it = m_actions.find(name);
    if (it != m_actions.end()) {
        return it->second();
    }
    return false;
}

bool Input::isActionJustPressed(const std::string& name) const {
    return false;
}

}
