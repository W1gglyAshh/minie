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
    std::vector<int> buffer_ids;
    int current_id;

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

    std::vector<int> cached_cx;
    std::vector<int> cached_cy;
    std::vector<int> cached_ox;
    std::vector<int> cached_oy;
    std::vector<std::string> cached_sm;
    std::vector<std::string> chached_fn;
    std::vector<bool> cached_mo;

    void updateScreen();
    void processKE(const KEVENT &e);
    void mvCursor(int dx, int dy);
    void scrollTFit();
    bool execCmd(const std::string &cmd);
    void toggleCmdP();
    void renderCmdP();

    void cacheBuffer(int id);
    void restoreBuffer(int id);

  public:
    Editor();
    ~Editor();

    bool init();
    void run();
    void oFile(const std::string &fn = "");
    bool sFile(const std::string &fn = "");
};
