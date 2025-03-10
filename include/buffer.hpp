#pragma once

#include <string>
#include <vector>

class TBuffer
{
  private:
    std::vector<std::string> lines;

  public:
    TBuffer();

    bool loadFFile(const std::string &fn);
    bool saveTFile(const std::string &fn);

    void insCh(int row, int col, char c);
    void delCh(int row, int col);
    void insLine(int row, const std::string &l);
    void delLine(int row);
    void splitLine(int row, int col);
    void joinLines(int row);

    size_t getLCount() const;
    size_t getLLength(int row) const;
    const std::string &getL(int row) const;
};
