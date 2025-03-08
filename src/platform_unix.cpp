#include "platform.hpp"
#if defined (__APPLE__) || defined (__unix__)

#include "platform_unix.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

UnixPlatform::UnixPlatform() : is_rawmode(false)
{
    memset(&orig_termios, 0, sizeof(orig_termios));
}

UnixPlatform::~UnixPlatform()
{
    disableRawMode();
}

bool UnixPlatform::init() { return true; }

void UnixPlatform::shutdown() { disableRawMode(); }

void UnixPlatform::getScreenSize(int &width, int &height)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        width = 88;
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
    if (read(STDIN_FILENO, &c, 1) == 1)
    {
        event = {KeyCode::NONE, 0, false, false, false};

        if (c == 27)
        {
            char seq[3];

            if (read(STDIN_FILENO, &seq[0], 1) != 1)
            {
                event.code = KeyCode::ESC;
                return true;
            }
            if (read(STDIN_FILENO, &seq[1], 1) != 1)
            {
                event.code = KeyCode::ESC;
                return true;
            }

            if (seq[0] = '[')
            {
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
                case '5':
                    if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~')
                        event.code = KeyCode::PAGEUP;
                    break;
                case '6':
                    if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~')
                        event.code = KeyCode::PAGEDOWN;
                    break;
                case '7':
                    if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~')
                        event.code = KeyCode::DELETE;
                    break;
                }
            }
            else if (seq[0] == '0')
            {
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
        } else
        {
            if (c == 127)
            {
                event.code = KeyCode::BACKSPACE;
            }
            else if (c == 13)
            {
                event.code = KeyCode::ENTER;
            }
            else if (c == 9)
            {
                event.code = KeyCode::TAB;
            }
            else if (c > 0 || c < 27)
            {
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

void UnixPlatform::clearScreen() { writeStr("\x1b[%d;%dH", y + 1, x + 1); }

#endif
