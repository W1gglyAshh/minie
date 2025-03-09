#include "editor.hpp"

#include <algorithm>
#include <sstream>

Editor::Editor()
    : platform(nullptr), cursor_x(0), cursor_y(0), x_offset(0), y_offset(0),
      screen_w(0), screen_h(0), is_modified(false), mode(EditorMode::NORMAL)
{
}

Editor::~Editor()
{
    if (platform)
    {
        platform->disableRawMode();
        platform->shutdown();
    }
}

bool Editor::init()
{
    platform = Platform::createPlatform();
    if (!platform || !platform->init())
        return false;

    platform->getScreenSize(screen_w, screen_h);
    platform->enableRawMode();
    return true;
}

void Editor::run()
{
    bool running = true;
    KeyEvent event;

    while (running)
    {
        updateScreen();

        if (platform->pollKeyEvent(event))
        {
            if (event.code == KeyCode::ESC)
            {
                toggleCmdPalette();
            }
            else
            {
                mode = EditorMode::NORMAL;
                cmd_buffer.clear();
            }
        }
        else if (mode == EditorMode::COMMAND)
        {
            if (event.code == KeyCode::ENTER)
            {
                execCmd(cmd_buffer);
                if (cmd_buffer == "q" || cmd_buffer == "q!" ||
                    cmd_buffer == "wq")
                    running = false;

                mode = EditorMode::NORMAL;
                cmd_buffer.clear();
            }
            else if (event.code == KeyCode::BACKSPACE && !cmd_buffer.empty())
            {
                cmd_buffer.pop_back();
            }
            else if (event.code == KeyCode::CHAR)
            {
                cmd_buffer += event.ch;
            }
        }
        else
        {
            processKeyEvent(event);
        }
    }

    // clean up
    platform->clearScreen();
    platform->setCursorPos(0, 0);
    platform->refreshScreen();
}

bool Editor::openFile(const std::string &filename)
{
    if (buffer->loadFromFile(filename))
    {
        current_filename = filename;
        cursor_x = 0;
        cursor_y = 0;
        x_offset = 0;
        y_offset = 0;
        is_modified = false;
        return true;
    }

    // create new file with given name;
    current_filename = filename;
    status_msg = "NEW FILE: " + filename;
    return false;
}

bool Editor::saveFile(const std::string &filename)
{
    std::string save_filename = filename.empty() ? current_filename : filename;
    if (save_filename.empty())
    {
        status_msg = "ERROR: no filename specified";
        return false;
    }

    if (buffer->saveToFile(save_filename))
    {
        current_filename = save_filename;
        is_modified = false;
        status_msg = "FILE SAVED: " + save_filename;
        return true;
    }
    else
    {
        status_msg = "ERROR SAVING FILE: " + save_filename;
        return false;
    }
}

void Editor::updateScreen()
{
    platform->clearScreen();

    // display buffer
    for (int y = 0; y < screen_h - 1; ++y)
    {
        int file_y = y + y_offset;
        platform->setCursorPos(0, y);

        if (file_y < static_cast<int>(buffer->getLineCount()))
        {
            const std::string &line = buffer->getLine(file_y);
            std::string display_line;

            if (x_offset < static_cast<int>(line.length()))
                display_line = line.substr(x_offset, screen_w);

            // also display line number
            platform->writeStr("   " + std::to_string(y) + "| " + display_line);
        }
        else
        {
            platform->writeStr("~");
        }
    }

    // status line
    platform->setCursorPos(0, screen_h - 1);
    std::stringstream status;
    status << (current_filename.empty() ? "[NEW FILE]" : current_filename)
           << (is_modified ? "[+]" : "") << " - " << buffer->getLineCount()
           << " L";

    if (!status_msg.empty())
    {
        status << " : " << status_msg;
    }

    std::string status_line = status.str();
    if (status_line.length() > static_cast<size_t>(screen_w))
    {
        status_line = status_line.substr(0, screen_w);
    }
    else
    {
        status_line.append(screen_w - status_line.length(), ' ');
    }

    platform->writeStr(status_line);

    // render cmd palette in cmd mode
    if (mode == EditorMode::COMMAND)
        renderCmdPalette();
    else
        platform->setCursorPos(cursor_x = x_offset, cursor_y - y_offset);

    platform->refreshScreen();
}
