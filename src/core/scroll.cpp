/* Copyright (C) 2025 Gabriel LU - Licensed under GPL v2 */

#include "core/editor.hpp"

void Editor::scrollTFit()
{
    avw = sw - LNW;

    int tvsl = 0;
    for (int i = 0; i < cy; i++)
    {
        int ll = tb.getLLength(i);
        if (ll <= ox)
        {
            tvsl += 1;
        }
        else
        {
            int vsl = ll - ox;
            tvsl += (vsl + avw - 1) / avw;
        }
    }

    int cfx = cx - LNW;
    int clw = cfx / avw;
    tvsl += clw;

    // horizontal
    int wrx = cfx % avw;
    if (wrx < ox)
        ox = wrx;
    else if (wrx >= ox + avw)
        ox = wrx - avw + 1;

    // vertical
    if (tvsl < oy)
        oy = tvsl;
    else if (tvsl >= oy + (sh - 1))
        oy = tvsl - (sh - 2);

    oy = std::max(0, oy);
}
