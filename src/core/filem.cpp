#include "core/editor.hpp"

void Editor::oFile(const std::string &fn)
{
    if (!fn.empty())
    {
        if (tb.loadFFile(fn))
        {
            current_fn = fn;
            // due to line number
            cx = tb.getLLength(0) + 7;
            cy = 0;
            ox = 0;
            oy = 0;
            mo = false;

            return;
        }

        // set current filename even if the file isn't loadable
        // for creating new file
        current_fn = fn;
        sm = "NEW FILE: " + fn;
    }
}

bool Editor::sFile(const std::string &fn)
{
    std::string sfn = fn.empty() ? current_fn : fn;
    if (sfn.empty())
    {
        sm = "\x1b[31mERROR: NO FILENAME SPECIFIED\x1b[30m";
        return false;
    }

    tb.insCh(tb.getLCount() - 1, tb.getLLength(tb.getLCount() - 1), '\n');

    if (tb.saveTFile(sfn))
    {
        current_fn = sfn;
        mo = false;
        sm = "FILE SAVED: " + sfn;
        return true;
    }
    else
    {
        sm = "\x1b[31mERROR SAVING FILE: \x1b[30m" + sfn;
        return false;
    }
}
