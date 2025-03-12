#include "editor.hpp"

#include <iostream>

int main(int argc, char **argv)
{
    Editor ed;

    if (!ed.init())
    {
        std::cerr << "Failed to initialize MINIE!\n";
        return -1;
    }

    if (argc == 2)
    {
        ed.oFile(argv[1]);
    }
    else if (argc == 1)
    {
        ed.oFile();
    }
    else
    {
        std::cout << "MINIE currently don't support opening multiple "
                     "files!\nThis feature will be add in the future.\n";
        return 0;
    }

    ed.run();
    return 0;
}
