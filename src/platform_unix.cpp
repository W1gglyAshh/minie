#if defined(__APPLE__) || defined(__unix__)

#include "platform_unix.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

UnixPl::UnixPl() : raw(false) { memset(&orig, 0, sizeof(orig)); }

UnixPl::~UnixPl() { disableRawM(); }

bool UnixPl::init() { return true; }

void UnixPl::shutdown() { disableRawM(); }

void UnixPl::getScreenSize(int &width, int &height)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        width = 80;
        height = 24;
    }
    else
    {
        width = ws.ws_col;
        height = ws.ws_row;
    }
}

bool UnixPl::pollKEvent(KEVENT &e)
{
    char ch;
    if (read(STDIN_FILENO, &ch, 1) == 1)
    {
        e = {KEY::NONE, 0, false, false, false};

        if (ch == 27)
        {
            char seq[3];

            if (read(STDIN_FILENO, &seq[0], 1) != 1)
            {
                e.k = KEY::ESC;
                return true;
            }
            if (read(STDIN_FILENO, &seq[1], 1) != 1)
            {
                e.k = KEY::ESC;
                return true;
            }

            if (seq[0] == '[')
            {
                switch (seq[1])
                {
                case 'A':
                    e.k = KEY::UP;
                    break;
                case 'B':
                    e.k = KEY::DOWN;
                    break;
                case 'C':
                    e.k = KEY::RIGHT;
                    break;
                case 'D':
                    e.k = KEY::LEFT;
                    break;
                }
            }
        }
        else
        {
            if (ch == 127)
            {
                e.k = KEY::BACKSPACE;
            }
            else if (ch == 13)
            {
                e.k = KEY::ENTER;
            }
            else if (ch == 9)
            {
                e.k = KEY::TAB;
            }
            else if (ch >= 0 && ch < 27)
            {
                e.k = KEY::CHAR;
                e.c = ch + 'a' - 1;
                e.ctrl = true;
            }
            else
            {
                e.k = KEY::CHAR;
                e.c = ch;
            }
        }
        return true;
    }
    return false;
}

void UnixPl::clrScreen() { writeStr("\x1b[2J\x1b[H"); }

void UnixPl::setCPos(int x, int y)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y + 1, x + 1);
    writeStr(buf);
}

void UnixPl::writeStr(const std::string &str)
{
    write(STDOUT_FILENO, str.c_str(), str.length());
}

// no refresh neede on unix terminals
void UnixPl::refreshScreen() {}

void UnixPl::enableRawM()
{
    if (raw)
        return;

    if (tcgetattr(STDIN_FILENO, &orig) == -1)
        return;

    struct termios r = orig;
    r.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    r.c_oflag &= ~(OPOST);
    r.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    r.c_cc[VMIN] = 0;
    r.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &r) < 0)
        return;

    raw = true;
}

void UnixPl::disableRawM()
{
    if (!raw)
        return;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    raw = false;
}

#endif
