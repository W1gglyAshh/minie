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

    if (argc > 1)
    {
        if (!ed.oFile(argv[1]))
            std::cerr << "Failed to open file: " << argv[1] << std::endl;
    }

    ed.run();
    return 0;
}
