/* Copyright (C) 2025 Gabriel LU - Licensed under GPL v2 */

#pragma once

#include "buffer.hpp"
#include "platform/platform.hpp"

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

    // line number, from thousand to hundred to ten, to single digits
    int lnk;
    int lnh;
    int lnt;
    int lns;

    void updateScreen();
    void processKE(const KEVENT &e);
    void mvCursor(int dx, int dy);
    void scrollTFit();
    bool execCmd(const std::string &cmd);
    void toggleCmdP();
    void renderCmdP();

    std::string calcLn(int t);

  public:
    Editor();
    ~Editor();

    bool init();
    void run();
    void oFile(const std::string &fn = "");
    bool sFile(const std::string &fn = "");
};
