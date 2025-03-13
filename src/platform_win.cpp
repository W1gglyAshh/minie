#ifdef _WIN32

#include "platform_win.hpp"

#include <cstdio>
#include <cstring>
#include <iostream>

WinPl::WinPl() : raw(false), mouse(false), asb(false)
{
    hin = GetStdHandle(STD_INPUT_HANDLE);
    hout = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleMode(hin, &oin);
    GetConsoleMode(hout, &oout);

    GetConsoleScreenBufferInfo(hout, &orig_csbi);
}

WinPl::~WinPl()
{
    disableRawM();
    disableMouse();
    disableASB();
}

bool WinPl::init()
{
    return (hin != INVALID_HANDLE_VALUE && hout != INVALID_HANDLE_VALUE);
}

void WinPl::shutdown()
{
    disableRawM();
    disableMouse();
    disableASB();
}

void WinPl::getScreenSize(int &width, int &height)
{
    if (GetConsoleScreenBufferInfo(hout, &csbi))
    {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
    else
    {
        width = 80;
        height = 24;
    }
}

bool WinPl::pollKEvent(KEVENT &e)
{
    e = {KEY::NONE, 0, false, false, false};

    INPUT_RECORD ir;
    DWORD evs;

    if (PeekConsoleInput(hin, &ir, 1, &evs) && evs > 0)
    {
        ReadConsoleInput(hin, &ir, 1, &evs);

        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown)
        {
            auto &key = ir.Event.KeyEvent;

            e.ctrl = (key.dwControlKeyState &
                      (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
            e.shift = (key.dwControlKeyState & SHIFT_PRESSED) != 0;
            e.alt = (key.dwControlKeyState &
                     (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;

            switch (key.wVirtualKeyCode)
            {
            case VK_RETURN:
                e.k = KEY::ENTER;
                break;
            case VK_BACK:
                e.k = KEY::BACKSPACE;
                break;
            case VK_DELETE:
                e.k = KEY::DELETE;
                break;
            case VK_LEFT:
                e.k = KEY::LEFT;
                break;
            case VK_RIGHT:
                e.k = KEY::RIGHT;
                break;
            case VK_UP:
                e.k = KEY::UP;
                break;
            case VK_DOWN:
                e.k = KEY::DOWN;
                break;
            case VK_TAB:
                e.k = KEY::TAB;
                break;
            case VK_ESCAPE:
                e.k = KEY::ESC;
                break;
            default:
                if (key.uChar.AsciiChar)
                {
                    e.k = KEY::CHAR;
                    e.c = key.uChar.AsciiChar;
                }
                else
                {
                    return false;
                }
                break;
            }
            return true;
        }
        else if (ir.EventType == MOUSE_EVENT)
        {
            auto &mve = ir.EventType.MouseEvent;

            if (mve.dwEventFlags == 0)
            {
                if (mve.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
                {
                    e.k = KEY::MOUSEDOWN;
                    return true;
                }
                else if ((mve.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) ==
                         0)
                {
                    static bool wdo = false;
                    if (wdo)
                    {
                        wdo = false;
                        e.k = KEY::MOUSEUP;
                        return true;
                    }
                    wdo = true;
                }
            }
        }
    }
    return false;
}

void WinPl::clrScreen()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cell_count;
    COORD hco = {0, 0};

    if (GetConsoleScreenBufferInfo(hout, &csbi))
    {
        cell_count = csbi.dwSize.X * csbi.dwSize.Y;

        FillConsoleOutputCharacter(hout, ' ', cell_count, hco, &count);
        FillConsoleOutputAttribute(hout, csbi.wAttributes, cell_count, hco,
                                   &count);
        SetConsoleCursorPosition(hout, hco);
    }
}

void WinPl::setCPos(int x, int y)
{
    COORD crd;
    crd.X = static_cast<SHORT>(x);
    crd.Y = static_cast<SHORT>(y);
    SetConsoleCursorPosition(hout, crd);
}

void WinPl::writeStr(const std::string &str)
{
    DWORD wrt;
    WriteConsole(hout, str.c_str(), static_cast<DWORD>(str.length()), &wrt,
                 nullptr);
}

// no need for refreshing screen in Windows
void WinPl::refreshScreen() {}

void WinPl::enableRawM()
{
    if (raw)
        return;

    DWORD mode = oin;
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    mode |= ENABLE_WINDOW_INPUT | ENABLE_VIRTUAL_TERMINAL_INPUT;

    if (SetConsoleMode(hin, mode))
        raw = true;
}

void WinPl::disableRawM()
{
    if (!raw)
        return;

    SetConsoleMode(hin, oin);
    raw = false;
}

void WinPl::enableMouse()
{
    if (mouse)
        return;

    DWORD mode = oin;
    mode |= ENABLE_MOUSE_INPUT;

    if (SetConsoleMode(hin, mode))
    {
        mouse = true;
    }
}

void WinPl::disableMouse()
{
    if (!mouse)
        return;

    DWORD mode = oin;
    mode &= ~ENABLE_MOUSE_INPUT;

    SetConsoleMode(hin, mode);
    mouse = false;
}

void WinPl::enableASB()
{
    if (asb)
        return;

    GetConsoleScreenBufferInfo(hin, &csbi);

    DWORD mode = oin;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (SetConsoleMode(hout, mode))
    {
        writeStr("\033[?1049h");
        asb = true;
    }
}

void WinPl::disableASB()
{
    if (!asb)
        return;

    writeStr("\033[?1049l");
    SetConsoleMode(hout, oin);

    asb = false;
}

#endif
