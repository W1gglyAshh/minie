#pragma once

#include <cstddef>
#include <string>
#include <vector>

class TextBuffer
{
  private:
    std::vector<std::string> lines;

  public:
    TextBuffer();

    // file loading
    bool loadFromFile(const std::string &filename);

    // file saving
    bool saveToFile(const std::string &filename);

    // editing operations
    void insertChar(int row, int col, char c);
    void deleteChar(int row, int col);
    void insertLine(int row, const std::string &line);
    void deleteLine(int row);
    void splitLine(int row, int col);
    void joinLines(int row);

    size_t getLineCount() const;
    const std::string &getLine(int row) const;
    size_t getLineLength(int row) const;
};
