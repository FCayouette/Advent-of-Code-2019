#include <iostream>
#include <fstream>
#include <string>
#include "TemplatedUtilities.h"
#include <vector>
#include <map>
#include <set>
#include <array>

constexpr bool IsPortal(char c) { return c >= 'A' && c <= 'Z'; }

struct Portal
{
    Portal(char c1 = '.', char c2 = '.') : name({ c1,c2 }) {}
    Portal(const std::array<char, 2>& a) : name(a) {}
    mutable Point A, B; // dirty trick to allow use in set (safe as they are not used in comparison)
    std::array<char, 2> name;
    bool operator<(const Portal& p) const { return name < p.name; }
};

int main(int argc, char*argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: Day20.exe [filename]" << std::endl;
        return -1;
    }

    std::fstream in(argv[1], std::ios::in);
    if (!in)
        return -1;

    std::vector<std::string> maze;
    std::string line;
    while (std::getline(in, line))
        maze.push_back(line);
    
    in.close();

    Point pMax(maze[0].size(), maze.size());
    std::set<Portal> portals;
    std::map<Point, std::array<char, 2>> pointsToPortal;

    // Scan 
    for (int i = 0; i < pMax.y; ++i)
        for (int j = 0; j < pMax.x; ++j)
        {
            char c = maze[i][j];
            if (IsPortal(c))
            {
                if (j + 1 < pMax.x && IsPortal(maze[i][j + 1]))
                {
                    Portal portal(c, maze[i][j + 1]);
                    Point pos;
                    if (j + 2 < pMax.x && maze[i][j + 2] == '.')
                        pos = Point(j + 2, i);
                    else
                        pos = Point(j - 1, i);

                    pointsToPortal[pos] = portal.name;
                    if (auto iter = portals.find(portal);
                        iter == portals.cend())
                    {
                        portal.A = pos;
                        portals.insert(portal);
                    }
                    else
                        iter->B = pos;
                    ++j;
                }
                else if (i + 1 < pMax.y && IsPortal(maze[i + 1][j]))
                {
                    Portal portal(c, maze[i+1][j]);
                    Point pos;
                    if (i + 2 < pMax.y && maze[i + 2][j] == '.')
                        pos = Point(j, i + 2);
                    else
                        pos = Point(j, i-1);
                    
                    pointsToPortal[pos] = portal.name;
                    if (auto iter = portals.find(portal);
                        iter == portals.cend())
                    {
                        portal.A = pos;
                        portals.insert(portal);
                    }
                    else
                        iter->B = pos;
                }
            }
        }

    Point target = portals.crbegin()->A;
    Point start = portals.cbegin()->A;
    portals.cbegin()->B = portals.cbegin()->A;
    portals.crbegin()->B = portals.crbegin()->A;
    Bounds bounds;
    bounds += pMax + Point(-1, -1);

    AStar<Point, int> aStar([target](const Point& p) { return ManhattanDistance(p, target); });
    aStar.EstimatedEmplace(start, 0);

    do
    {
        AStar<Point, int>::ASI currentInfo = aStar.PopFront();
        const int nextCost = currentInfo.cost + 1;
        for (int i = 0; i < 4; ++i)
        {
            Point np = currentInfo.info + directions[i];
            if (np == target)
            {
                std::cout << "Part 1: " << nextCost << std::endl;
                goto AStarCompleted; // We are done, get out
            }
            if (bounds.Inside(np) && !aStar.HasEvaluatedNode(np) && maze[np.y][np.x] == '.')
                aStar.EstimatedEmplace(np, nextCost);
        }
        if (auto portal = pointsToPortal.find(currentInfo.info);
            portal != pointsToPortal.cend())
        {
            auto iter = portals.find(portal->second);
            if (iter->A == currentInfo.info)
            {
                if (!aStar.HasEvaluatedNode(iter->B))
                    aStar.EstimatedEmplace(iter->B, nextCost);
            }
            else if (!aStar.HasEvaluatedNode(iter->A))
                aStar.EstimatedEmplace(iter->A, nextCost);
        }
    } while (!aStar.Empty());
AStarCompleted:

    // Going 3D for part 2 solution
    // Make sure Portals have A in the outside and B in the inside
    for (auto iter = portals.cbegin(); iter != portals.cend(); ++iter)
    {
        Point b = iter->B;
        if (b.y == 2 || b.x == 2 || b.y >= maze.size() - 4 || b.x >= maze[0].size() - 4)
            std::swap(iter->A, iter->B);
    }

    // Compute minimal cost to change layer
    int minCost = bounds.maxX - bounds.minX + bounds.maxY - bounds.minY;
    for (auto iter = portals.cbegin(); iter != portals.cend(); ++iter)
    {
        int cost = ManhattanDistance(target, iter->B);
        if (cost < minCost && cost != 0)
            minCost = cost;
    }
    Point3D target3d(target.x, target.y, 0);
    Point3D start3d(start.x, start.y, 0);
    constexpr std::array<char, 2> entrance{ 'A', 'A' }, exit{ 'Z','Z' };

    AStar<Point3D, int> aStar3d([minCost, target](const Point3D& p) { return ManhattanDistance(target, Point(p.x, p.y)) + minCost * p.z; });
    aStar3d.EstimatedEmplace(start3d, 0);

    do
    {
        AStar<Point3D, int>::ASI currentInfo = aStar3d.PopFront();
        const int nextCost = currentInfo.cost + 1;
        for (int i = 0; i < 4; ++i)
        {
            Point3D np = currentInfo.info + flatDirections[i];
            if (np == target3d)
            {
                std::cout << "Part 2: " << nextCost << std::endl;
                goto AStar3DCompleted; // We are done, get out
            }
            if (bounds.Inside(np.Flatten()) && !aStar3d.HasEvaluatedNode(np) && maze[np.y][np.x] == '.')
                aStar3d.EstimatedEmplace(np, nextCost);            
        }
        if (auto portal = pointsToPortal.find(currentInfo.info.Flatten());
            portal != pointsToPortal.cend())
        {
            if (currentInfo.info.z != 0 && (portal->second == entrance || portal->second == exit))
                continue; // Not at the correct level for entrance and exit
            auto iter = portals.find(portal->second);
            if (iter->A == currentInfo.info.Flatten())
            {
                if (currentInfo.info.z == 0) continue;
                Point3D inner(iter->B.x, iter->B.y, currentInfo.info.z - 1);
                if (!aStar3d.HasEvaluatedNode(inner))
                    aStar3d.EstimatedEmplace(inner, nextCost);
            }
            else
            {
                Point3D outer(iter->A.x, iter->A.y, currentInfo.info.z + 1);
                if (!aStar3d.HasEvaluatedNode(outer))
                    aStar3d.EstimatedEmplace(outer, nextCost);
            }
        }
    } while (!aStar3d.Empty());

AStar3DCompleted:
    return 0;
}