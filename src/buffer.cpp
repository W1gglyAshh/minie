#include "buffer.hpp"

#include <fstream>

TextBuffer::TextBuffer()
{
    // start with an empty line
    lines.push_back("");
}

bool TextBuffer::loadFromFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    lines.clear();
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);

    // at least one line exist
    if (lines.empty())
        lines.push_back("");

    return true;
}

bool TextBuffer::saveToFile(const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
        return false;

    for (size_t i = 0; i < lines.size(); ++i)
    {
        file << lines[i];
        if (i < lines.size() - 1)
            file << "\n";
    }

    return true;
}

void TextBuffer::insertChar(int row, int col, char c)
{
    if (row >= 0 && row < static_cast<int>(lines.size()) && col >= 0 &&
        col <= static_cast<int>(lines[row].length()))
        lines[row].insert(col, 1, c);
}

void TextBuffer::deleteChar(int row, int col)
{
    if (row >= 0 && row < static_cast<int>(lines.size()) && col >= 0 &&
        col < static_cast<int>(lines[row].length()))
        lines[row].erase(col, 1);
}

void TextBuffer::insertLine(int row, const std::string &line)
{
    if (row >= 0 && row <= static_cast<int>(lines.size()))
        lines.insert(lines.begin() + row, line);
}

void TextBuffer::deleteLine(int row)
{
    if (row >= 0 && row < static_cast<int>(lines.size()))
    {
        lines.erase(lines.begin() + row);

        // at least one line exist
        if (lines.empty())
            lines.push_back("");
    }
}

void TextBuffer::splitLine(int row, int col)
{
    if (row >= 0 && row < static_cast<int>(lines.size()) && col >= 0 &&
        col <= static_cast<int>(lines[row].length()))
    {
        std::string new_line = lines[row].substr(col);
        lines[row].erase(col);
        insertLine(row + 1, new_line);
    }
}

void TextBuffer::joinLines(int row)
{
    if (row >= 0 && row < static_cast<int>(lines.size()) - 1)
    {
        lines[row] += lines[row + 1];
        deleteLine(row + 1);
    }
}

size_t TextBuffer::getLineCount() const { return lines.size(); }

const std::string &TextBuffer::getLine(int row) const
{
    static std::string empty;
    if (row >= 0 && row < static_cast<int>(lines.size()))
        return lines[row];

    return empty;
}

size_t TextBuffer::getLineLength(int row) const
{
    if (row >= 0 && row < static_cast<int>(lines.size()))
        return lines[row].length();

    return 0;
}
