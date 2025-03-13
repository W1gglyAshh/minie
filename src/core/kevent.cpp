/* Copyright (C) 2025 Gabriel LU - Licensed under GPL v2 */

#include "core/editor.hpp"

void Editor::processKE(const KEVENT &e)
{
    if (e.k == KEY::CHAR)
    {
        tb.insCh(cy, cx - 7, e.c);
        cx++;
        mo = true;
    }
    else if (e.k == KEY::ENTER)
    {
        tb.splitLine(cy, cx - 7);
        cx = 7;
        cy++;
        mo = true;
    }
    else if (e.k == KEY::TAB)
    {
        for (int i = 0; i < 4; i++)
        {
            tb.insCh(cy, cx - 7, ' ');
            cx++;
        }
        mo = true;
    }
    else if (e.k == KEY::BACKSPACE)
    {
        if (cx > 7)
        {
            cx--;
            tb.delCh(cy, cx - 7);
            mo = true;
        }
        else if (cy > 0)
        {
            cx = tb.getLLength(cy - 1) + 7;
            tb.joinLines(cy - 1);
            cy--;
            mo = true;
        }
    }
    else if (e.k == KEY::LEFT)
    {
        if (cx > 7)
        {
            mvCursor(-1, 0);
        }
        else if (cy > 0 && cx <= 7)
        {
            cy--;
            cx = tb.getLLength(cy) + 7;
            scrollTFit();
        }
    }
    else if (e.k == KEY::RIGHT)
    {
        int mx = tb.getLLength(cy) + 7;
        if (cx < mx)
        {
            mvCursor(1, 0);
        }
        else if (cy < tb.getLCount() - 1)
        {
            cy++;
            cx = 7;
            scrollTFit();
        }
    }
    else if (e.k == KEY::UP)
    {
        mvCursor(0, -1);
        if (cx < 7)
            cx = 7;
    }
    else if (e.k == KEY::DOWN)
    {
        mvCursor(0, 1);
        if (cx < 7)
            cx = 7;
    }
    // mouse scroll function is temporarily removed due to an issue

    scrollTFit();
}

void Editor::mvCursor(int dx, int dy)
{
    int avw = sw - 7;
    int cfx = cx - 7;

    if (dy != 0)
    {
        cy += dy;
        cy = std::max(0, std::min(cy, static_cast<int>(tb.getLCount() - 1)));

        int ll = tb.getLLength(cy);
        if (cfx > ll)
            cx = ll + 7;
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

        if (cx < 7)
            cx = 7;
    }

    scrollTFit();
}
