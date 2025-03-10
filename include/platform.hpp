#pragma once

#include <string>

enum class KEY
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
    TAB,
    ESC
};

struct KEVENT
{
    KEY k;
    char c;
    bool ctrl;
    bool shift;
    bool alt;
};

class Platform
{
  public:
    virtual ~Platform() = default;

    virtual bool init() = 0;
    virtual void shutdown() = 0;

    virtual void getScreenSize(int &width, int &height) = 0;
    virtual bool pollKEvent(KEVENT &e) = 0;

    virtual void clrScreen() = 0;
    virtual void setCPos(int x, int y) = 0;
    virtual void writeStr(const std::string &str) = 0;
    virtual void refreshScreen() = 0;

    virtual void enableRawM() = 0;
    virtual void disableRawM() = 0;

    static std::unique_ptr<Platform> createPl();
};
