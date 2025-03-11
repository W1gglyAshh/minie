#pragma once

#include "buffer.hpp"
#include "platform.hpp"

#include <string>

enum class EMode
{
    NOR,
    CMD
};

class Editor
{
  private:
    std::unique_ptr<Platform> pl;
    TBuffer tb;

    int cx, cy;
    int ox, oy;
    int sw, sh;
    std::string sm;
    std::string current_fn;
    bool mo;

    EMode mode;
    std::string cbf;

    void updateScreen();
    void processKE(const KEVENT &e);
    void mvCursor(int dx, int dy);
    void scrollTFit();
    bool execCmd(const std::string &cmd);
    void toggleCmdP();
    void renderCmdP();

  public:
    Editor();
    ~Editor();

    bool init();
    void run();
    bool oFile(const std::string &fn);
    bool sFile(const std::string &fn = "");
};
