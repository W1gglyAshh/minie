#pragma once

#include "platform.hpp"
#include "buffer.hpp"

#include <string>
#include <vector>
#include <memory>

enum class EditorMode
{
    NORMAL,
    COMMAND
};

class Editor
{
private:
    std::unique_ptr<Platform> platform;
    std::unique_ptr<TextBuffer> buffer;
    EditorMode mode;

    int cursor_x, cursor_y;
    int x_offset, y_offset;
    int screen_w, screen_h;

    std::string status_msg;
    std::string cmd_buffer;
    std::string current_filename;

    bool is_modified;

    // private member functions
    void updateScreen();
    void processKeyEvent(const KeyEvent &event);
    void moveCursor(int dx, int dy);
    void scrollToFit();
    void execCmd(const std::string &cmd);
    void toggleCmdPalette();
    void renderCmdPalette();

public:
    Editor();
    ~Editor();

    bool init();
    void run();
    bool openFile(const std::string &filename);
    bool saveFile(const std::string &filename = "");
};
