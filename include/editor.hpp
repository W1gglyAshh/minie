#pragma once

#include "buffer.hpp"
#include "platform.hpp"

#include <map>
#include <string>
#include <vector>

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

    std::map<int, int> cached_cx;
    std::map<int, int> cached_cy;
    std::map<int, int> cached_ox;
    std::map<int, int> cached_oy;
    std::map<int, std::string> cached_sm;
    std::map<int, std::string> cached_fn;
    std::map<int, bool> cached_mo;

    std::map<int, std::vector<std::string>> cached_buffers;

    void updateScreen();
    void processKE(const KEVENT &e);
    void mvCursor(int dx, int dy);
    void scrollTFit();
    bool execCmd(const std::string &cmd);
    void toggleCmdP();
    void renderCmdP();

    void cacheBuffer(int id);
    void restoreBuffer(int id);
    void enableTab();
    void disableTab();

  public:
    Editor();
    ~Editor();

    bool init();
    void run();
    void oFile(const std::string &fn = "");
    bool sFile(const std::string &fn = "");
};
