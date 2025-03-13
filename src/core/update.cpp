#include "core/editor.hpp"

#include <sstream>

void Editor::updateScreen()
{
    pl->clrScreen();
    pl->getScreenSize(sw, sh);

    // byte count
    int bc = 0;

    // for line wrapping
    int avw = sw - 8;
    int ddl = 0;

    // calc visible lines
    int vslc = 0;
    int lgl = 0;
    int vslo = 0;

    // logical display
    for (int i = 0; i < static_cast<int>(tb.getLCount()) && vslc <= oy; ++i)
    {
        const std::string &l = tb.getL(i);
        int ll = l.length();

        int wlc = 1;
        if (ll > ox)
            wlc = (ll - ox + avw - 1) / avw;

        if (vslc + wlc > oy)
        {
            lgl = i;
            vslo = oy - vslc;
            break;
        }
        vslc += wlc;
    }

    // display buffer
    for (int fli = lgl; fli < static_cast<int>(tb.getLCount()) && ddl < sh - 1;
         ++fli)
    {
        const std::string &cul = tb.getL(fli);
        int ll = cul.length();

        int nwl = 1;
        if (ll > ox)
            nwl = (ll - ox + avw - 1) / avw;
        else
            nwl = 1;

        int startWsg = (fli == lgl) ? vslo : 0;

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
        vslo = 0;
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
