#include <iostream>
#include <fstream>
#include <string>
#include "TemplatedUtilities.h"
#include <vector>
#include <set>
#include <map>

using Area = std::vector<std::string>;
using uint = unsigned int;
using Biodiversities = std::set<uint>;
using ErisArea = std::map<int, Area>;
constexpr Point center(2, 2);

uint ComputeBiodiversity(const Area& area)
{
    uint value = 1, result = 0;
    for (size_t i = 0; i < area.size(); ++i)
        for (size_t j = 0; j < area[i].size(); ++j)
        {
            if (area[i][j] == '#')
                result += value;
            value <<= 1;
        }
    return result;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage Day24.exe [filename]" << std::endl;
        return -1;
    }
    std::ifstream in(argv[1], std::ios::in);
    if (!in)
        return -1;
    
    std::string line;
    Area area, work;
    while (in >> line)
        area.push_back(line);
    in.close();

    work = area;
    ErisArea eris;
    eris[0] = area; // Create Part 2 data before work on part 1

    Biodiversities biodiversities;
    Bounds bounds;
    bounds += Point(area.size() - 1, area[0].size() - 1);
    while (true)
    {
        uint result = ComputeBiodiversity(area);
        if (biodiversities.find(result) != biodiversities.cend())
        {
            std::cout << "Part 1: " << result << std::endl;
            break;
        }
        biodiversities.insert(result);

        for(size_t x = 0; x < area.size(); ++x)
            for (size_t y = 0; y < area[x].size(); ++y)
            {
                // Count adjacents;
                Point p(x, y);
                int count = 0;
                for (int i = 0; i < 4; ++i)
                {
                    Point np = p + directions[i];
                    if (bounds.Inside(np) && area[np.x][np.y] == '#')
                        ++count;
                }
                if (area[x][y] == '#')
                    work[x][y] = (count == 1 ? '#' : '.');
                else
                    work[x][y] = (count == 1 || count == 2) ? '#' : '.';
            }

        std::swap(area, work);
    }

    Area emptyRecursive;
    for (int i = 0; i < 5; ++i)
        emptyRecursive.emplace_back(i == 2 ? "..?.." : ".....");

    ErisArea erisWork;
    erisWork = eris;
    
    int minLevel = 0, maxLevel = 0;
    for (int minutes = 0; minutes < 200; ++minutes)
    {
        // Check if we need to add new levels
        const Area& evalMin = eris[minLevel];
        int count = 0;
        for (int i = 1; i < 4; ++i)
            for (int j = 1; j < 4; ++j)
            {
                if (i == 2 && j == 2)
                    continue;
                if (evalMin[i][j] == '#')
                    ++count;
            }
        if (count > 0)
        {
            eris[--minLevel] = emptyRecursive;
            erisWork[minLevel] = emptyRecursive;
        }

        const Area& evalMax = eris[maxLevel];
        for (int i = 0; i < 5; ++i)
            if (evalMax[i][0] == '#' || evalMax[0][i] == '#' || evalMax[i][4] == '#' || evalMax[4][i] == '#')
            {
                eris[++maxLevel] = emptyRecursive;
                erisWork[maxLevel] = emptyRecursive;
                break;
            }

        // Start evaluation
        for (int level = minLevel; level <= maxLevel; ++level)
        {
            for (int x = 0; x < 5; ++x)
                for (int y = 0; y < 5; ++y)
                {
                    Point p(x, y);
                    if (p == center)
                        continue;
                    int count = 0;

                    for (int i = 0; i < 4; ++i)
                    {
                        Point np = p + directions[i];
                        if (!bounds.Inside(np)) // Need to check upper level
                        {
                            if (level != maxLevel)
                            {
                                if (np.x < 0 && eris[level + 1][1][2] == '#')
                                    ++count;
                                else if (np.y < 0 && eris[level + 1][2][1] == '#')
                                    ++count;
                                else if (np.x > 4 && eris[level + 1][3][2] == '#')
                                    ++count;
                                else if (np.y > 4 && eris[level + 1][2][3] == '#')
                                    ++count;
                            }
                        }
                        else if (np == center) // Need to check lower level
                        {
                            if (level != minLevel)
                            {
                                if (p == Point(1, 2))
                                {
                                    for (int y = 0; y < 5; ++y)
                                        if (eris[level - 1][0][y] == '#')
                                            ++count;
                                } 
                                else if (p == Point(2, 1))
                                {
                                    for (int x = 0; x < 5; ++x)
                                        if (eris[level - 1][x][0] == '#')
                                            ++count;
                                }
                                else if (p == Point(2, 3))
                                {
                                    for (int x = 0; x < 5; ++x)
                                        if (eris[level - 1][x][4] == '#')
                                            ++count;
                                }
                                else 
                                    for (int y = 0; y < 5; ++y)
                                        if (eris[level - 1][4][y] == '#')
                                            ++count;
                            }
                        }
                        else if (eris[level][np.x][np.y] == '#')
                                ++count;
                    }

                    if (eris[level][x][y] == '#')
                        erisWork[level][x][y] = (count == 1 ? '#' : '.');
                    else
                        erisWork[level][x][y] = (count == 1 || count == 2) ? '#' : '.';
                }
        }
        std::swap(eris, erisWork);
    }

    int bugCount = 0;
    for (const auto& level : eris)
    {
        const Area& a = level.second;
        for (int i = 0; i < 5; ++i)
            for (int j = 0; j < 5; ++j)
                if (a[i][j] == '#')
                    ++bugCount;
    }
    std::cout << "Part 2: " << bugCount << std::endl;
}