/* Copyright (C) 2025 Gabriel LU - Licensed under GPL v2 */

#include "core/editor.hpp"

void Editor::toggleCmdP()
{
    mode = EMode::CMD;
    cbf.clear();
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
