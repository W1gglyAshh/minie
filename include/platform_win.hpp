#pragma once

#ifdef _WIN32

#include "platform.hpp"

#include <conios>
#include <window.h>

class WinPl : public Platform
{
  private:
    HANDLE hin;
    HANDLE hout;

    DWORD oin;
    DWORD oout;

    bool raw;
    bool mouse;
    bool asb;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    CONSOLE_SCREEN_BUFFER_INFO orig_csbi;

  public:
    WinPl();
    ~WinPl();

    bool init() override;
    void shutdown() override;

    void getScreenSize(int &width, int &height) override;
    bool pollKEvent(KEVENT &e) override;
    void clrScreen() override;

    void setCPos(int x, int y) override;
    void writeStr(const std::string &str) override;
    void refreshScreen() override;

    void enableRawM() override;
    void disableRawM() override;

    void enableMouse() override;
    void disableMouse() override;

    void enableASB() override;
    void disableASB() override;
};

#endif
