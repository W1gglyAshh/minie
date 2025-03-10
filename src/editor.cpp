#include "editor.hpp"

#include <algorithm>
#include <sstream>

Editor::Editor()
    : pl(nullptr), cx(0), cy(0), ox(0), oy(0), sw(0), sh(0), mo(false),
      mode(EMode::NOR)
{
}

Editor::~Editor()
{
    if (pl)
    {
        pl->disableRawM();
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
                    execCmd(cbf);
                    if (cbf == "q" || cbf == "q!" || cbf == "wq")
                        r = false;
                    mode = EMode::NOR;
                    cbf.clear();
                }
            }
        }
    }
}
