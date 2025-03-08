#pragma once

#if defined (__APPLE__) || defined (__unix__)

#include "platform.hpp"

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

class UnixPlatform : public Platform
{
private:
    struct termios orig_termios;
    bool is_rawmode;
    
public:
    UnixPlatform();
    ~UnixPlatform();

    bool init() override;
    void shutdown() override;

    void getScreenSize(int &width, int &height) override;

    // input
    bool pollKeyEvent(KeyEvent &event) override;

    // output
    void clearScreen() override;
    void setCursorPos(int x, int y) override;
    void writeStr(const std::string &str) override;
    void refreshScreen() override;

    void enableRawMode() override;
    void disableRawMode() override;
};

#endif
