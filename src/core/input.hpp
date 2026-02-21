#pragma once

#include "types.hpp"
#include <array>
#include <bitset>
#include <functional>

namespace viber {

enum class KeyCode {
    Unknown = 0,
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Space = 32,
    Enter = 257,
    Escape = 256,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Grave = 96,
    Minus = 45,
    Equal = 61,
    LeftBracket = 91,
    RightBracket = 93,
    Backslash = 92,
    Semicolon = 59,
    Apostrophe = 39,
    Comma = 44,
    Period = 46,
    Slash = 47,
};

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
};

enum class GamepadButton {
    A = 0,
    B = 1,
    X = 2,
    Y = 3,
    LeftBumper = 4,
    RightBumper = 5,
    Back = 6,
    Start = 7,
    Guide = 8,
    LeftThumb = 9,
    RightThumb = 10,
    DPadUp = 11,
    DPadRight = 12,
    DPadDown = 13,
    DPadLeft = 14,
};

struct GamepadState {
    std::array<float, 6> axes{};
    std::bitset<15> buttons;
    bool connected = false;
};

class Input {
public:
    Input() = default;
    
    void update();
    
    void setKeyState(int key, bool pressed);
    void setMouseButtonState(int button, bool pressed);
    void setMousePosition(float x, float y);
    void setMouseDelta(float dx, float dy);
    
    bool isKeyPressed(KeyCode key) const;
    bool isKeyJustPressed(KeyCode key) const;
    bool isKeyJustReleased(KeyCode key) const;
    
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonJustPressed(MouseButton button) const;
    bool isMouseButtonJustReleased(MouseButton button) const;
    
    vec2 getMousePosition() const { return m_mousePosition; }
    vec2 getMouseDelta() const { return m_mouseDelta; }
    
    float getMouseScroll() const { return m_mouseScroll; }
    void addMouseScroll(float delta) { m_mouseScroll += delta; }
    
    const GamepadState& getGamepad(int index = 0) const;
    void updateGamepad(int index);
    
    void setAction(const std::string& name, KeyCode key);
    void setAction(const std::string& name, MouseButton button);
    void setAction(const std::string& name, GamepadButton button, int gamepadIndex = 0);
    
    bool isActionPressed(const std::string& name) const;
    bool isActionJustPressed(const std::string& name) const;
    
private:
    std::bitset<512> m_keys;
    std::bitset<512> m_keysPrevious;
    std::bitset<8> m_mouseButtons;
    std::bitset<8> m_mouseButtonsPrevious;
    
    vec2 m_mousePosition{0.0f};
    vec2 m_mouseDelta{0.0f};
    float m_mouseScroll = 0.0f;
    
    std::array<GamepadState, 4> m_gamepads;
    
    std::unordered_map<std::string, std::function<bool()>> m_actions;
};

}
