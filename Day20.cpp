#include <iostream>
#include <fstream>
#include <string>
#include "TemplatedUtilities.h"
#include <vector>
#include <map>
#include <set>
#include <array>
#include <cassert>

constexpr bool IsPortal(char c) { return c >= 'A' && c <= 'Z'; }

struct Portal
{
    Portal() : name({ '.','.' }) {};
    Portal(char c1, char c2) : name({ c1,c2 }) {}
    mutable Point A, B; // dirty trick to allow use in set (safe as they are not used in comparison)
    std::array<char, 2> name;
    bool operator<(const Portal& p)const { return name < p.name; }
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
                    {
                        assert(j > 0 && maze[i][j - 1] == '.');
                        pos = Point(j - 1, i);
                    }
                    pointsToPortal[pos] = { c, portal.name[1] };
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
                    {
                        assert(i > 0 && maze[i - 1][j] == '.');
                        pos = Point(j, i-1);
                    }
                    pointsToPortal[pos] = { c, portal.name[1] };
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
    std::set<Point> testedPoints;
    testedPoints.insert(start);
    std::set<Point> previousIter = testedPoints;
    Bounds bounds;
    bounds += pMax + Point(-1, -1);
    int iterations = 0;
    bool found = false;
    do
    {
        ++iterations;
        std::set<Point> currentIteration;

        for (const Point& p : previousIter)
        {
            for (int i = 0; i < 4; ++i)
            {
                Point np = p + directions[i];
                if (np == target)
                {
                    found = true;
                    break;
                }
                if (bounds.Inside(np) && testedPoints.find(np) == testedPoints.cend() && maze[np.y][np.x] == '.')
                {
                    testedPoints.insert(np);
                    currentIteration.insert(np);
                }
            }
            if (found)
                break;
            if (auto portal = pointsToPortal.find(p);
                portal != pointsToPortal.cend())
            {
                auto iter = portals.find(Portal(portal->second[0], portal->second[1]));
                assert(iter != portals.cend());
                if (iter->A == p)
                {
                    if (testedPoints.find(iter->B) == testedPoints.cend())
                    {
                        testedPoints.insert(iter->B);
                        currentIteration.insert(iter->B);
                    }
                }
                else if (testedPoints.find(iter->A) == testedPoints.cend())
                {
                    testedPoints.insert(iter->A);
                    currentIteration.insert(iter->A);
                }
            }
        }
        if (found)
            break;
        std::swap(currentIteration, previousIter);
    } while (!previousIter.empty());


    std::cout << "Part 1: " << iterations << std::endl;

    // Going 3D in the solution
    // Make sure Portals have A in the outside and B in the inside
    
    for (auto iter = portals.cbegin(); iter != portals.cend(); ++iter)
    {
        Point b = iter->B;
        if (b.y == 2 || b.x == 2 || b.y >= maze.size() - 4 || b.x >= maze[0].size() - 4)
            std::swap(iter->A, iter->B);
    }

    Point3D target3d(target.x, target.y, 0);
    Point3D start3d(start.x, start.y, 0);
    std::set<Point3D> tested3Dpoints;
    tested3Dpoints.insert(start3d);
    std::set<Point3D> previous3DIter = tested3Dpoints;
    iterations = 0;
    found = false;
    do
    {
        ++iterations;
        std::set<Point3D> currentIteration;

        for (const Point3D& p : previous3DIter)
        {
            for (int i = 0; i < 4; ++i)
            {
                Point3D np = p + flatDirections[i];
                if (np == target3d)
                {
                    found = true;
                    break;
                }
                if (bounds.Inside(Point(np.x, np.y)) && tested3Dpoints.find(np) == tested3Dpoints.cend() && maze[np.y][np.x] == '.')
                {
                    tested3Dpoints.insert(np);
                    currentIteration.insert(np);
                }
            }
            if (found)
                break;
            Point p2D(p.x, p.y);
            if (auto portal = pointsToPortal.find(p2D);
                portal != pointsToPortal.cend())
            {
                if (p.z != 0 && ((portal->second[0] == 'A' && portal->second[1]=='A') || 
                    (portal->second[0] == 'Z' && portal->second[1] == 'Z')))
                    continue; // Not at the correct level for entrance and exit
                auto iter = portals.find(Portal(portal->second[0], portal->second[1]));
                assert(iter != portals.cend());
                if (iter->A == p2D)
                {
                    if (p.z == 0) continue;
                    Point3D inner(iter->B.x, iter->B.y, p.z - 1);
                    if (tested3Dpoints.find(inner) == tested3Dpoints.cend())
                    {
                        tested3Dpoints.insert(inner);
                        currentIteration.insert(inner);
                    }
                }
                else
                {
                    Point3D outer(iter->A.x, iter->A.y, p.z + 1);
                    if (tested3Dpoints.find(outer) == tested3Dpoints.cend())
                    {
                        tested3Dpoints.insert(outer);
                        currentIteration.insert(outer);
                    }
                }
            }
        }
        if (found)
            break;
        std::swap(currentIteration, previous3DIter);
    } while (!previousIter.empty());

    std::cout << "Part 2: " << iterations << std::endl;
}