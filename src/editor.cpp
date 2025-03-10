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

bool Editor::oFile(const std::string &fn)
{
    if (tb.loadFFile(fn))
    {
        current_fn = fn;
        cx = 0;
        cy = 0;
        ox = 0;
        oy = 0;
        mo = false;

        return true;
    }

    // set current filename even if the file isn't loadable
    // for creating new file
    current_fn = fn;
    sm = "NEW FILE: " + fn;
    return false;
}

bool Editor::sFile(const std::string &fn)
{
    std::string sfn = fn.empty() ? current_fn : fn;
    if (sfn.empty())
    {
        sm = "ERROR: NO FILENAME SPECIFIED";
        return false;
    }

    tb.insCh(tb.getLCount() - 1, tb.getLLength(tb.getLCount() - 1), '\n');

    if (tb.saveTFile(sfn))
    {
        current_fn = sfn;
        mo = false;
        sm = "FILE SAVED: " + sfn;
        return true;
    }
    else
    {
        sm = "ERROR SAVING FILE: " + sfn;
        return false;
    }
}

void Editor::updateScreen()
{
    pl->clrScreen();

    // display buffer
    for (int y = 0; y < sh - 1; ++y)
    {
        int fy = y + oy;
        pl->setCPos(0, y);

        if (fy < static_cast<int>(tb.getLCount()))
        {
            const std::string &l = tb.getL(fy);
            std::string dl;

            if (ox < static_cast<int>(l.length()))
                dl = l.substr(ox, sw);

            pl->writeStr(dl);
        }
        else
        {
            pl->writeStr("~");
        }
    }

    // display status line
    pl->setCPos(0, sh - 1);
    std::stringstream ss;
    ss << (current_fn.empty() ? "[NEW FILE]" : current_fn) << (mo ? "[+]" : "")
       << " - " << cy + 1 << " Ln " << cx + 1 << " Col";

    if (!sm.empty())
        ss << " | " << sm;

    std::string ssl = ss.str();
    if (ssl.length() > static_cast<size_t>(sw))
        ssl = ssl.substr(0, sw);
    else
        ssl.append(sw - ssl.length(), ' ');

    pl->writeStr(ssl);

    if (mode == EMode::CMD)
        renderCmdP();
    else
        pl->setCPos(cx - ox, cy - oy);

    pl->refreshScreen();
}

void Editor::renderCmdP()
{
    // calculate position
    int pw = std::min(40, sw - 4);
    int px = (sw - pw) / 2;
    int py = 2;

    // draw background
    for (int y = py; y < py + 3; ++y)
    {
        pl->setCPos(px - 1, y);
        std::string l(pw + 2, ' ');
        pl->writeStr(l);
    }

    // border
    pl->setCPos(px - 1, py - 1);
    pl->writeStr("+" + std::string(pw, '-') + "+");

    pl->setCPos(px - 1, py);
    pl->writeStr("|");
    pl->setCPos(px + pw, py);
    pl->writeStr("|");

    pl->setCPos(px - 1, py + 1);
    pl->writeStr("+" + std::string(pw, '-') + "+");

    // command prompt
    pl->setCPos(px, py);
    std::string pt = ":" + cbf;
    if (pt.length() > static_cast<size_t>(pw))
        pt = pt.substr(pt.length() - pw);
    pl->writeStr(pt);

    // position cursor at end of command
    pl->setCPos(px + 1 + cbf.length(), py);
}

void Editor::processKE(const KEVENT &e)
{
    if (e.k == KEY::CHAR)
    {
        tb.insCh(cy, cx, e.c);
        cx++;
        mo = true;
    }
    else if (e.k == KEY::ENTER)
    {
        tb.splitLine(cy, cx);
        cx = 0;
        cy++;
        mo = true;
    }
    else if (e.k == KEY::TAB)
    {
        for (int i = 0; i < 4; i++)
        {
            tb.insCh(cy, cx, ' ');
            cx++;
        }
        mo = true;
    }
    else if (e.k == KEY::BACKSPACE)
    {
        if (cx > 0)
        {
            cx--;
            tb.delCh(cy, cx);
            mo = true;
        }
        else if (cy > 0)
        {
            cx = tb.getLLength(cy - 1);
            tb.joinLines(cy - 1);
            cy--;
            mo = true;
        }
    }
    else if (e.k == KEY::LEFT)
    {
        mvCursor(-1, 0);
    }
    else if (e.k == KEY::RIGHT)
    {
        mvCursor(1, 0);
    }
    else if (e.k == KEY::UP)
    {
        mvCursor(0, -1);
    }
    else if (e.k == KEY::DOWN)
    {
        mvCursor(0, 1);
    }

    scrollTFit();
}

void Editor::mvCursor(int dx, int dy)
{
    cx += dx;
    cy += dy;

    if (cy < 0)
        cy = 0;
    else if (cy >= static_cast<int>(tb.getLCount()))
        cy = tb.getLCount() - 1;

    if (cx < 0)
    {
        cx = 0;
    }
    else
    {
        int mx = tb.getLLength(cy);
        if (cx > mx)
            cx = mx;
    }
}

void Editor::scrollTFit()
{
    // horizontal scrolling
    if (cx < ox)
    {
        ox = cx;
    }
    else if (cx >= ox + sw)
    {
        ox = cx - sw + 1;
    }

    // vertical scrolling
    if (cy < oy)
    {
        oy = cy;
    }
    else if (cy >= oy + sh - 1)
    {
        oy = cy - sh + 2;
    }
}

void Editor::toggleCmdP()
{
    mode = EMode::CMD;
    cbf.clear();
}

void Editor::execCmd(const std::string &cmd)
{
    if (cmd == "w")
    {
        sFile();
    }
    else if (cmd.substr(0, 2) == "w ")
    {
        std::string fn = cmd.substr(2);
        sFile(fn);
    }
    else if (cmd == "q")
    {
        if (mo)
            sm = "NO WRITE SINCE LAST CHANGE (ADD ! TO OVERRIDE)";
    }
    else if (cmd == "q!")
    {
        // handled in run
    }
    else if (cmd == "wq")
    {
        sFile();
    }
    else
    {
        sm = "ERROR: UNKNOWN COMMAND: " + cmd;
    }
}
