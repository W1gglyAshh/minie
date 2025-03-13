/* Copyright (C) 2025 Gabriel LU - Licensed under GPL v2 */

#include "core/editor.hpp"

void Editor::processKE(const KEVENT &e)
{
    bool ctc = false;

    if (e.k == KEY::CHAR)
    {
        tb.insCh(cy, cx - LNW, e.c);
        cx++;
        ctc = true;
    }
    else if (e.k == KEY::ENTER)
    {
        tb.splitLine(cy, cx - LNW);
        cx = LNW;
        cy++;
        ctc = true;
        nfr = true;
    }
    else if (e.k == KEY::TAB)
    {
        for (int i = 0; i < 4; i++)
        {
            tb.insCh(cy, cx - LNW, ' ');
            cx++;
        }
        ctc = true;
    }
    else if (e.k == KEY::BACKSPACE)
    {
        if (cx > LNW)
        {
            cx--;
            tb.delCh(cy, cx - LNW);
            ctc = true;
        }
        else if (cy > 0)
        {
            cx = tb.getLLength(cy - 1) + LNW;
            tb.joinLines(cy - 1);
            cy--;
            ctc = true;
            nfr = true;
        }
    }
    else if (e.k == KEY::LEFT)
    {
        if (cx > LNW)
        {
            mvCursor(-1, 0);
        }
        else if (cy > 0 && cx <= LNW)
        {
            cy--;
            cx = tb.getLLength(cy) + LNW;
            scrollTFit();
        }
    }
    else if (e.k == KEY::RIGHT)
    {
        int mx = tb.getLLength(cy) + LNW;
        if (cx < mx)
        {
            mvCursor(1, 0);
        }
        else if (cy < tb.getLCount() - 1)
        {
            cy++;
            cx = LNW;
            scrollTFit();
        }
    }
    else if (e.k == KEY::UP)
    {
        mvCursor(0, -1);
        if (cx < LNW)
            cx = LNW;
    }
    else if (e.k == KEY::DOWN)
    {
        mvCursor(0, 1);
        if (cx < LNW)
            cx = LNW;
    }
    // mouse scroll function is temporarily removed due to an issue

    scrollTFit();

    mo = ctc;
}

void Editor::mvCursor(int dx, int dy)
{
    avw = sw - LNW;
    int cfx = cx - LNW;

    if (dy != 0)
    {
        cy += dy;
        cy = std::max(0, std::min(cy, static_cast<int>(tb.getLCount() - 1)));

        int ll = tb.getLLength(cy);
        if (cfx > ll)
            cx = ll + LNW;
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

        if (cx < LNW)
            cx = LNW;
    }

    scrollTFit();
}
