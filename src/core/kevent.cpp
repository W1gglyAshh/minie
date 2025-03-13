#include "core/editor.hpp"

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
