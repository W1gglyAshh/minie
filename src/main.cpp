#include "editor.hpp"

#include <iostream>

int main(int argc, char **argv)
{
    Editor editor;

    if (!editor.init())
    {
        std::cerr << "Failed to initialize MINIE!" << std::endl;
        return -1;
    }

    if (argc > 1)
    {
        if (!editor.openFile(argv[1]))
        {
            std::cerr << "Failed to open file: " << argv[1] << std::endl;
        }
    }

    editor.run();

    return 0;
}
