#if defined(__APPLE__) || defined(__unix__)

#include "platform_unix.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

UnixPlatform::UnixPlatform() : is_rawmode(false)
{
    memset(&orig_termios, 0, sizeof(orig_termios));
}

UnixPlatform::~UnixPlatform() { disableRawMode(); }

bool UnixPlatform::init() { return true; }

void UnixPlatform::shutdown() { disableRawMode(); }

void UnixPlatform::getScreenSize(int &width, int &height)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        width = 80;
        height = 24;
    }
    else
    {
        width = ws.ws_row;
        height = ws.ws_col;
    }
}

bool UnixPlatform::pollKeyEvent(KeyEvent &event)
{
    char c;
    event = {KeyCode::NONE, 0, false, false, false};

    if (read(STDIN_FILENO, &c, 1) == 1)
    {
        if (c == 27)
        {
            char seq[3];

            // set a timeout for reading ESC sequence
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            // 100ms timeout
            struct timeval tv = {0, 100000};

            if (select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &tv) <= 0)
            {
                event.code = KeyCode::ESC;
                return true;
            }
            if (read(STDIN_FILENO, &seq[0], 1) != 1)
            {
                event.code = KeyCode::ESC;
                return true;
            }

            if (seq[0] == '[')
            {
                if (read(STDIN_FILENO, &seq[0], 1) != 1)
                {
                    event.code = KeyCode::ESC;
                    return true;
                }

                switch (seq[1])
                {
                case 'A':
                    event.code = KeyCode::UP;
                    break;
                case 'B':
                    event.code = KeyCode::DOWN;
                    break;
                case 'C':
                    event.code = KeyCode::RIGHT;
                    break;
                case 'D':
                    event.code = KeyCode::LEFT;
                    break;
                case 'H':
                    event.code = KeyCode::HOME;
                    break;
                case 'F':
                    event.code = KeyCode::END;
                    break;
                case '3':
                    if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~')
                        event.code = KeyCode::DELETE;
                    break;
                case '5':
                    if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~')
                        event.code = KeyCode::PAGEUP;
                    break;
                case '6':
                    if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~')
                        event.code = KeyCode::PAGEDOWN;
                    break;
                    // case '7':
                    //     if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] ==
                    //     '~')
                    //         event.code = KeyCode::DELETE;
                    //     break;
                }
            }
            else if (seq[0] == '0')
            {
                if (read(STDIN_FILENO, &seq[1], 1) != 1)
                {
                    event.code = KeyCode::ESC;
                    return true;
                }

                switch (seq[1])
                {
                case 'H':
                    event.code = KeyCode::HOME;
                    break;
                case 'F':
                    event.code = KeyCode::END;
                    break;
                }
            }
            else
            {
                event.code = KeyCode::ESC;
            }
        }
        else
        {
            if (c == 127)
            {
                event.code = KeyCode::BACKSPACE;
            }
            else if (c == 13 || c == 10)
            {
                event.code = KeyCode::ENTER;
            }
            else if (c == 9)
            {
                event.code = KeyCode::TAB;
            }
            else if (c < 32)
            {
                if (c == 0)
                    return false;

                event.code = KeyCode::CHAR;
                event.ch = c + 'a' - 1;
                event.ctrl = true;
            }
            else
            {
                event.code = KeyCode::CHAR;
                event.ch = c;
            }
        }

        return true;
    }

    return false;
}

void UnixPlatform::clearScreen() { writeStr("\x1b[2J\x1b[H"); }

void UnixPlatform::setCursorPos(int x, int y)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y + 1, x + 1);
    writeStr(buf);
}

void UnixPlatform::writeStr(const std::string &str)
{
    write(STDOUT_FILENO, str.c_str(), str.length());
}

void UnixPlatform::refreshScreen()
{
    // no explicit refresh needed on unix
}

void UnixPlatform::enableRawMode()
{
    if (is_rawmode)
        return;

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        return;

    struct termios raw = orig_termios;

    // input flags
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // output flag
    raw.c_oflag &= ~(OPOST);

    // control flag
    raw.c_cflag &= ~(CS8);

    // local flags
    // echo off, canonical off, no extended functions, no signal chars
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // immediate return
    raw.c_cc[VMIN] = 0;
    // 100ms timeout
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0)
        return;

    is_rawmode = true;
}

void UnixPlatform::disableRawMode()
{
    if (!is_rawmode)
        return;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    is_rawmode = false;
}

#endif
