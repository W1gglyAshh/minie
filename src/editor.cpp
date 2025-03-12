#include "editor.hpp"

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>

Editor::Editor()
    : pl(nullptr), cx(8), cy(0), ox(0), oy(0), sw(0), sh(0), mo(false),
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

void Editor::oFile(const std::string &fn)
{
    if (!fn.empty())
    {
        if (tb.loadFFile(fn))
        {
            current_fn = fn;
            // due to line number
            cx = tb.getLLength(0) + 8;
            cy = 0;
            ox = 0;
            oy = 0;
            mo = false;

            return;
        }

        // set current filename even if the file isn't loadable
        // for creating new file
        current_fn = fn;
        sm = "NEW FILE: " + fn;
    }
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
    pl->getScreenSize(sw, sh);

    // byte count
    int bc = 0;

    // for line wrapping
    int avw = sw - 8;
    int ddl = 0;

    // display buffer
    for (int fli = oy; fli < static_cast<int>(tb.getLCount()) && ddl < sh - 1;
         ++fli)
    {
        const std::string &cul = tb.getL(fli);
        int ll = cul.length();

        int nwl = 1;
        if (ll > ox)
            nwl = (ll - ox + avw - 1) / avw;
        else
            nwl = 1;

        for (int wsg = 0; wsg < nwl && ddl < sh - 1; ++wsg)
        {
            pl->setCPos(0, ddl);

            if (wsg == 0)
                pl->writeStr("\x1b[90m" + calcLn(fli + 1) + "\x1b[0m");
            else
                pl->writeStr("     ");

            int sp = ox + (wsg * avw);

            std::string dsg = "";
            if (sp < ll)
            {
                int sgl = std::min(avw, ll - sp);
                dsg = cul.substr(sp, sgl);
            }

            pl->writeStr(dsg);
            ddl++;
        }
    }

    while (ddl < sh - 1)
    {
        pl->setCPos(0, ddl);
        pl->writeStr("\x1b[2m~\x1b[0m");
        ddl++;
    }

    // calculate the byte size of the file
    for (int n = 0; n < static_cast<int>(tb.getLCount()); n++)
    {
        bc += tb.getLLength(n);
    }

    // display status line
    pl->setCPos(0, sh - 1);
    std::stringstream ss;
    ss << " " << (current_fn.empty() ? "[NEW FILE]" : current_fn)
       << (mo ? "[+]" : "") << " - " << cy + 1 << " Ln " << cx + 1 << " Col, "
       << bc << " B";

    if (!sm.empty())
        ss << " | " << sm;

    std::string ssl = ss.str();
    if (ssl.length() > static_cast<size_t>(sw))
        ssl = ssl.substr(0, sw);
    else
        ssl.append(sw - ssl.length(), ' ');

    // set alternate background and foreground color
    pl->writeStr("\x1b[107;30m" + std::string(sw, ' ') + "\x1b[0m");
    pl->setCPos(0, sh - 1);
    pl->writeStr("\x1b[107;30m" + ssl + "\x1b[0m");

    if (mode == EMode::CMD)
    {
        renderCmdP();
    }
    else
    {
        int cfx = cx - 8;
        int cfy = cy;
        int avw = sw - 8;

        int sy = 0;
        for (int i = oy; i < cfy; i++)
        {
            int ll = tb.getLLength(i);
            if (ll <= ox)
            {
                sy += 1;
            }
            else
            {
                int vsl = ll - ox;
                sy += (vsl + avw - 1) / avw;
            }
        }

        int clo = cfx - ox;
        if (clo >= 0)
        {
            sy += clo / avw;
            int sx = clo % avw;

            if (clo < avw)
                sx += 8;
            else
                sx += 5;

            pl->setCPos(sx, sy);
        }
    }

    pl->refreshScreen();
}

void Editor::renderCmdP()
{
    // calculate position
    int pw = std::min(40, sw - 4);
    int px = (sw - pw) / 2;
    int py = 2;

    std::string cpt = " COMMAND PALETTE ";
    std::string bs = "─";
    std::string ubsr;
    std::string ubsl;
    std::string bbs;

    int pus = (pw / 2.0f) - (cpt.length() / 2.0f);

    for (int i = 0; i < pus; i++)
        ubsr += bs;
    ubsl = ubsr + "─";
    for (int i = 0; i < pw; i++)
        bbs += bs;

    // draw background
    for (int y = py; y < py + 3; ++y)
    {
        pl->setCPos(px - 1, y);
        std::string l(pw + 2, ' ');
        pl->writeStr(l);
    }

    // border
    pl->setCPos(px - 1, py - 1);
    pl->writeStr("╭" + ubsl + cpt + ubsr + "╮");

    pl->setCPos(px - 1, py);
    pl->writeStr("│");
    pl->setCPos(px + pw, py);
    pl->writeStr("│");

    pl->setCPos(px - 1, py + 1);
    pl->writeStr("╰" + bbs + "╯");

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
        tb.insCh(cy, cx - 8, e.c);
        cx++;
        mo = true;
    }
    else if (e.k == KEY::ENTER)
    {
        tb.splitLine(cy, cx - 8);
        cx = 8;
        cy++;
        mo = true;
    }
    else if (e.k == KEY::TAB)
    {
        for (int i = 0; i < 4; i++)
        {
            tb.insCh(cy, cx - 8, ' ');
            cx++;
        }
        mo = true;
    }
    else if (e.k == KEY::BACKSPACE)
    {
        if (cx > 8)
        {
            cx--;
            tb.delCh(cy, cx - 8);
            mo = true;
        }
        else if (cy > 0)
        {
            cx = tb.getLLength(cy - 1) + 8;
            tb.joinLines(cy - 1);
            cy--;
            mo = true;
        }
    }
    else if (e.k == KEY::LEFT)
    {
        if (cx > 8)
        {
            mvCursor(-1, 0);
        }
        else if (cy > 0 && cx <= 8)
        {
            cy--;
            cx = tb.getLLength(cy) + 8;
            scrollTFit();
        }
    }
    else if (e.k == KEY::RIGHT)
    {
        int mx = tb.getLLength(cy) + 8;
        if (cx < mx)
        {
            mvCursor(1, 0);
        }
        else if (cy < tb.getLCount() - 1)
        {
            cy++;
            cx = 8;
            scrollTFit();
        }
    }
    else if (e.k == KEY::UP)
    {
        mvCursor(0, -1);
        if (cx < 8)
            cx = 8;
    }
    else if (e.k == KEY::DOWN)
    {
        mvCursor(0, 1);
        if (cx < 8)
            cx = 8;
    }
    // mouse scroll function is temporarily removed due to an issue

    scrollTFit();
}

void Editor::mvCursor(int dx, int dy)
{
    int avw = sw - 8;
    int cfx = cx - 8;

    if (dy != 0)
    {
        cy += dy;
        cy = std::max(0, std::min(cy, static_cast<int>(tb.getLCount() - 1)));

        int ll = tb.getLLength(cy);
        if (cfx > ll)
            cx = ll + 8;
    }

    if (dx != 0)
    {
        if (dx < 0 && cfx > 0)
        {
            cx += dx;
        }
        else if (dx > 0)
        {
            int mx = tb.getLLength(cy);
            if (cfx < mx)
                cx += dx;
        }

        if (cx < 8)
            cx = 8;
    }

    scrollTFit();
}

void Editor::scrollTFit()
{
    int avw = sw - 8;

    int cfx = cx - 8;
    int csy = 0;

    for (int i = 0; i < cy; i++)
    {
        int ll = tb.getLLength(i);
        int wls = std::max(1, (ll + avw - 1) / avw);
        csy += wls;
    }

    csy += cfx / avw;

    // horizontal
    int wrx = cfx % avw;
    if (wrx < ox)
        ox = wrx;
    else if (wrx >= ox + avw)
        ox = wrx - avw + 1;

    // vertical
    if (csy < oy)
        oy = csy;
    else if (csy >= oy + sh - 1)
        oy = csy - sh + 2;
}

void Editor::toggleCmdP()
{
    mode = EMode::CMD;
    cbf.clear();
}

bool Editor::execCmd(const std::string &cmd)
{
    if (cmd == "w" || cmd == "s")
    {
        sFile();
    }
    else if (cmd.substr(0, 2) == "w " || cmd.substr(0, 2) == "s ")
    {
        std::string fn = cmd.substr(2);
        std::erase(fn, ' ');
        sFile(fn);
    }
    else if (cmd == "q")
    {
        if (mo)
        {
            sm = "\x1b[31mNO WRITE SINCE LAST CHANGE (ADD ! TO "
                 "OVERRIDE)\x1b[30m";
            return true;
        }
        return false;
    }
    else if (cmd == "q!")
    {
        return false;
    }
    else if (cmd == "wq" || cmd == "sq")
    {
        sFile();
        return false;
    }
    else if (cmd.substr(0, 3) == "wq " || cmd.substr(0, 3) == "sq ")
    {
        std::string fn = cmd.substr(3);
        std::erase(fn, ' ');
        sFile(fn);
        return false;
    }
    else
    {
        sm = "\x1b[31mERROR: UNKNOWN COMMAND: " + cmd + "\x1b[30m";
    }
    return true;
}

std::string Editor::calcLn(int t)
{
    if (t < 10)
        return "    " + std::to_string(t) + " │ ";
    else if (t < 100 && t >= 10)
        return "   " + std::to_string(t) + " │ ";
    else if (t < 1000 && t >= 100)
        return "  " + std::to_string(t) + " │ ";
    else if (t < 10000 && t >= 1000)
        return " " + std::to_string(t) + " │ ";
    else
        return std::to_string(t) + " │ ";
    // I don’t accept files larger than 10k lines because only a stupid ass
    // would write something like that
}
