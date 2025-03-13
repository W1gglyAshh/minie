#include "core/editor.hpp"

#include <string>

Editor::Editor()
    : pl(nullptr), cx(7), cy(0), ox(0), oy(0), sw(0), sh(0), mo(false),
      mode(EMode::NOR), lnk(0), lnh(0), lns(1)
{
}

Editor::~Editor()
{
    if (pl)
    {
        pl->disableRawM();
        pl->disableMouse();
        // pl->disableASB();
        pl->shutdown();
    }
}

bool Editor::init()
{
    pl = Platform::createPl();
    if (!pl || !pl->init())
        return false;

    pl->getScreenSize(sw, sh);
    pl->enableRawM();
    // pl->enableMouse();
    pl->enableASB();
    return true;
}

void Editor::run()
{
    bool r = true;
    KEVENT ev;

    while (r)
    {
        updateScreen();

        if (pl->pollKEvent(ev))
        {
            if (ev.k == KEY::ESC)
            {
                if (mode == EMode::NOR)
                {
                    toggleCmdP();
                }
                else
                {
                    mode = EMode::NOR;
                    cbf.clear();
                }
            }
            else if (mode == EMode::CMD)
            {
                if (ev.k == KEY::ENTER)
                {
                    r = execCmd(cbf);
                    mode = EMode::NOR;
                    cbf.clear();
                }
                else if (ev.k == KEY::BACKSPACE && !cbf.empty())
                {
                    cbf.pop_back();
                }
                else if (ev.k == KEY::CHAR)
                {
                    cbf += ev.c;
                }
            }
            else
            {
                processKE(ev);
            }
        }
    }

    // cleanup
    pl->clrScreen();
    pl->setCPos(0, 0);
    pl->refreshScreen();
}

std::string Editor::calcLn(int t)
{
    if (t < 10)
        return "    " + std::to_string(t) + "  ";
    else if (t < 100 && t >= 10)
        return "   " + std::to_string(t) + "  ";
    else if (t < 1000 && t >= 100)
        return "  " + std::to_string(t) + "  ";
    else if (t < 10000 && t >= 1000)
        return " " + std::to_string(t) + "  ";
    else
        return std::to_string(t) + "  ";
    // I don’t accept files larger than 10k lines because only a stupid ass
    // would write something like that
}
