#pragma once

#ifdef _WIN32

#include "platform.hpp"

#include <windows.h>

class WinPlatform : public Platform
{
private:
    HANDLE h_stdin;
    HANDLE h_stdout;
    DWORD orig_in_mode;
    DWORD orig_out_mode;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

public:
    WinPlatform();
    ~WinPlatform();

    bool init() override;
    void shutdown() override;

    void getWindowSize(int &width, int &height) override;

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
