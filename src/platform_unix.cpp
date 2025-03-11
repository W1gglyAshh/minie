#if defined(__APPLE__) || defined(__unix__)

#include "platform_unix.hpp"

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

UnixPl::UnixPl() : raw(false), mouse(false), asb(false) { memset(&orig, 0, sizeof(orig)); }

UnixPl::~UnixPl()
{
    disableRawM();
    disableMouse();
    disableASB();
}

bool UnixPl::init() { return true; }

void UnixPl::shutdown()
{
    disableRawM();
    disableMouse();
    disableASB();
}

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
            char seq[16];

            if (read(STDIN_FILENO, &seq[0], 1) != 1)
            {
                e.k = KEY::ESC;
                return true;
            }

            if (seq[0] == '[')
            {
                if (read(STDIN_FILENO, &seq[1], 1) != 1)
                {
                    e.k = KEY::ESC;
                    return true;
                }

                // handle mouse
                if (seq[1] == 'M' || seq[1] == '<')
                {
                    // old style
                    if (seq[1] == 'M')
                    {
                        char mi[3];
                        if (read(STDIN_FILENO, mi, 3) != 3)
                            return false;

                        int bk = mi[0] - 32;
                        if ((bk & 0x40) != 0)
                        {
                            if ((bk & 0x1) != 0)
                            {
                                e.k = KEY::MOUSEUP;
                                return true;
                            }
                            else
                            {
                                e.k = KEY::MOUSEDOWN;
                                return true;
                            }
                        }
                        return false;
                    }
                    else if (seq[1] == '<')
                    {
                        // new style
                        char mi[32];
                        int idx = 0;

                        while (idx < 31)
                        {
                            if (read(STDIN_FILENO, &mi[idx], 1) != 1)
                                return false;

                            if (mi[idx] == 'm' || mi[idx] == 'M')
                                break;

                            idx++;
                        }
                        mi[idx + 1] = '\0';

                        int bk;
                        sscanf(mi, "%d", &bk);

                        if (bk == 64 || bk == 65)
                        {
                            e.k = KEY::MOUSEUP;
                            return true;
                        }
                        else if (bk == 66 || bk == 67)
                        {
                            e.k = KEY::MOUSEDOWN;
                            return true;
                        }
                        return false;
                    }
                }
                else
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

// no refresh needed on unix terminals
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

void UnixPl::enableMouse()
{
    if (mouse)
        return;

    printf("\033[?1000h");
    printf("\033[?1002h");
    printf("\033[?1006h");
    printf("\033[?1007h");
    mouse = true;
}

void UnixPl::disableMouse()
{
    if (!mouse)
        return;

    printf("\033[?1000l");
    printf("\033[?1002l");
    printf("\033[?1006l");
    printf("\033[?1007l");
    mouse = false;
}

void UnixPl::enableASB()
{
    // alternate screen buffer
    if (asb)
        return;

    printf("\033[?1049h");
    fflush(stdout);
    asb = true;
}

void UnixPl::disableASB()
{
    if (!asb)
        return;

    printf("\033[?1049l");
    fflush(stdout);
    asb = false;
}

#endif
