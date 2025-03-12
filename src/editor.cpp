#include "editor.hpp"

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>

Editor::Editor()
    : current_id(0), pl(nullptr), cx(0), cy(0), ox(0), oy(0), sw(0), sh(0),
      mo(false), mode(EMode::NOR)
{
    buffer_ids.clear();
}

Editor::~Editor()
{
    if (pl)
    {
        pl->disableRawM();
        pl->disableMouse();
        pl->disableASB();
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
    pl->enableMouse();
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
            cx = tb.getLLength(0);
            cy = 0;
            ox = 0;
            oy = 0;
            mo = false;
        }

        // set current filename even if the file isn't loadable
        // for creating new file
        current_fn = fn;
        sm = "NEW FILE: " + fn;
    }

    // store the current buffer id
    current_id += 1;
    buffer_ids.push_back(current_id);
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
    int bc;

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
    // calculate the byte size of the file
    bc = 0;
    for (int n = 0; n < static_cast<int>(tb.getLCount()); n++)
    {
        bc += tb.getLLength(n);
    }

    // display status line
    pl->setCPos(0, sh - 1);
    std::stringstream ss;
    ss << (current_fn.empty() ? "[NEW FILE]" : current_fn) << (mo ? "[+]" : "")
       << " - " << cy + 1 << " Ln " << cx + 1 << " Col, " << bc << " B";

    if (!sm.empty())
        ss << " | " << sm;

    std::string ssl = ss.str();
    if (ssl.length() > static_cast<size_t>(sw))
        ssl = ssl.substr(0, sw);
    else
        ssl.append(sw - ssl.length(), ' ');

    // set alternate background and foreground color
    printf("\x1b[47;30m");
    fflush(stdout);
    pl->writeStr(ssl);
    printf("\x1b[0m");
    fflush(stdout);

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
    else if (e.k == KEY::MOUSEUP)
    {
        if (oy > 0)
        {
            oy--;
        }
        return;
    }
    else if (e.k == KEY::MOUSEDOWN)
    {
        if (oy < tb.getLCount() - (sh - 1))
        {
            oy++;
            if (cy <= 0)
                cy = 0;
        }
        return;
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

bool Editor::execCmd(const std::string &cmd)
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
        {
            sm = "NO WRITE SINCE LAST CHANGE (ADD ! TO OVERRIDE)";
            return true;
        }
        // not quitting the loop but quit the current buffer
        // TODO
        return false;
    }
    else if (cmd == "q!")
    {
        return false;
        // TODO
    }
    else if (cmd == "wq")
    {
        sFile();
        return false;
    }
    else if (cmd.substr(0, 2) == "o ")
    {
        std::string nfn = cmd.substr(2);
        cacheBuffer(current_id);
        current_id += 1;
        buffer_ids.push_back(current_id);

        if (buffer_ids.size() > 1)
            enableTab();
    }
    else if (cmd.empty())
    {
        return true;
    }
    else
    {
        sm = "ERROR: UNKNOWN COMMAND: " + cmd;
    }
    return true;
}

void Editor::cacheBuffer(int id)
{
    cached_cx[id] = cx;
    cached_cy[id] = cy;
    cached_ox[id] = ox;
    cached_oy[id] = oy;
    cached_fn[id] = current_fn;
    cached_sm[id] = sm;
    cached_mo[id] = mo;

    cx = 0;
    cy = 0;
    ox = 0;
    oy = 0;
    current_fn.clear();
    sm.clear();
    mo = false;
}

void Editor::restoreBuffer(int id)
{
    auto it = std::find(buffer_ids.begin(), buffer_ids.end(), id);
    if (it == buffer_ids.end())
        return;

    cacheBuffer(current_id);

    current_id = id;
    cx = cached_cx[id];
    cy = cached_cy[id];
    ox = cached_ox[id];
    oy = cached_oy[id];
    current_fn = cached_fn[id];
    sm = cached_sm[id];
    mo = cached_mo[id];
}
