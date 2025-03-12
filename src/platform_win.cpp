#ifdef _WIN32

#include "platform_win.hpp"

#include <cstdio>
#include <cstring>
#include <iostream>

WinPl::WinPl() : raw(false), mouse(false), asb(false)
{
    hin = GetStdHandle(STD_INPUT_HANDLE);
    hout = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleMode(hin, &oin);
    GetConsoleMode(hout, &oout);

    GetConsoleScreenBufferInfo(hout, &orig_csbi);
}

#endif
