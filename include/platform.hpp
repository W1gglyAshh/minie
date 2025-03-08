#pragma once

#include <functional>
#include <string>

enum class KeyCode
{
    NONE,
    CHAR,
    ENTER,
    BACKSPACE,
    DELETE,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    HOME,
    END,
    PAGEUP,
    PAGEDOWN,
    TAB,
    ESC
};

struct KeyEvent
{
    KeyCode code;
    char ch;
    bool shift;
    bool ctrl;
    bool alt;    
};

class Platform
{
public:
    virtual ~Platform() = default;

    virtual bool init() = 0;
    virtual void shutdown() = 0;

    virtual void getScreenSize(int &width, int &height) = 0;

    // raw input handling
    virtual bool pollKeyEvent(KeyEvent &event) = 0;

    // raw output handling
    virtual void clearScreen() = 0;
    virtual void setCursorPos(int x, int y) = 0;
    virtual void writeStr(const std::string &str) = 0;
    virtual void refreshScreen() = 0;

    virtual void enableRawMode() = 0;
    virtual void disableRawMode() = 0;

    static Platform *createPlatform();
};
