#include "buffer.hpp"

#include <fstream>

TBuffer::TBuffer()
{
    // start with an empty line
    lines.push_back("");
}

bool TBuffer::loadFFile(const std::string &fn)
{
    std::ifstream file(fn);
    if (!file.is_open())
        return false;

    lines.clear();
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);

    if (lines.empty())
        lines.push_back("");

    return true;
}

bool TBuffer::saveTFile(const std::string &fn)
{
    std::ofstream file(fn);
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

void TBuffer::insCh(int row, int col, char c)
{
    if (row >= 0 && row < static_cast<int>(lines.size()) && col >= 0 &&
        col <= static_cast<int>(lines[row].length()))
        lines[row].insert(col, 1, c);
}

void TBuffer::delCh(int row, int col)
{
    if (row >= 0 && row < static_cast<int>(lines.size()) && col >= 0 &&
        col < static_cast<int>(lines[row].length()))
        lines[row].erase(col, 1);
}

void TBuffer::insLine(int row, const std::string &line)
{
    if (row >= 0 && row <= static_cast<int>(lines.size()))
        lines.insert(lines.begin() + row, line);
}

void TBuffer::delLine(int row)
{
    if (row >= 0 && row < static_cast<int>(lines.size()))
    {
        lines.erase(lines.begin() + row);

        // ensure at least one row exist
        if (lines.empty())
            lines.push_back("");
    }
}

void TBuffer::splitLine(int row, int col)
{
    if (row >= 0 && row < static_cast<int>(lines.size()) && col >= 0 &&
        col <= static_cast<int>(lines[row].length()))
    {
        std::string new_l = lines[row].substr(col);
        lines[row].erase(col);
        insLine(row + 1, new_l);
    }
}

void TBuffer::joinLines(int row)
{
    if (row >= 0 && row < static_cast<int>(lines.size()) - 1)
    {
        lines[row] += lines[row + 1];
        delLine(row + 1);
    }
}

size_t TBuffer::getLCount() const { return lines.size(); }

size_t TBuffer::getLLength(int row) const
{
    if (row >= 0 && row < static_cast<int>(lines.size()))
        return lines[row].length();
    return 0;
}

const std::string &TBuffer::getL(int row) const
{
    static std::string emp;
    if (row >= 0 && row < static_cast<int>(lines.size()))
        return lines[row];
    return emp;
}
