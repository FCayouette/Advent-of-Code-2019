#include <iostream>
#include <fstream>
#include <string>
#include "TemplatedUtilities.h"
#include <map>
#include <tuple>

constexpr bool IsPortal(char c) { return c >= 'A' && c <= 'Z'; }
constexpr std::array<char, 2> entrancePortal{ 'A', 'A' }, exitPortal{ 'Z','Z' };

struct Portal
{
    Portal(char c1 = '.', char c2 = '.') : name({ c1,c2 }) {}
    Portal(const std::array<char, 2>& a) : name(a) {}
    mutable Point A, B; // dirty trick to allow use in set (safe as they are not used in comparison)
    std::array<char, 2> name;
    bool operator<(const Portal& p) const { return name < p.name; }
};

enum class Position : char
{
    inner,
    outer
};
constexpr Position StepThrough(Position p) { return p == Position::inner ? Position::outer : Position::inner; }

using PortalInfo = std::tuple<std::array<char, 2>, Position>;
using PortalAndDistance = std::tuple<std::array<char, 2>, Position, int>;
struct Connections
{
    constexpr Connections(char c1 = '.', char c2 = '.', Position p = Position::inner) : portal({ c1, c2 }, p) {}
    constexpr Connections(const std::array<char, 2>& a, bool inner) : portal(a, inner ? Position::inner : Position::outer) {}
    constexpr Connections(const PortalInfo& pi) : portal(pi) {}
    PortalInfo portal;
    std::vector<PortalAndDistance> connectsTo;

    constexpr bool operator<(const Connections& c) const { return portal < c.portal; }
};

using Portals = std::set<Portal>;
using PointsToPortal = std::map<Point, std::array<char, 2>>;

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
    Portals portals;
    PointsToPortal pointsToPortal;

    // Scan for portals
    {
        auto PortalInsert = [&pointsToPortal, &portals](Portal& portal, Point pos) {
            pointsToPortal[pos] = portal.name;
            if (auto iter = portals.find(portal);
                iter == portals.cend())
            {
                portal.A = pos;
                portals.insert(portal);
            }
            else
                iter->B = pos;
        };

        for (int i = 0; i < pMax.y; ++i)
            for (int j = 0; j < pMax.x; ++j)
                if (char c = maze[i][j];
                    IsPortal(c))
                {
                    if (j + 1 < pMax.x && IsPortal(maze[i][j + 1]))
                    {
                        Portal portal(c, maze[i][j + 1]);
                        Point pos = (j + 2 < pMax.x && maze[i][j + 2] == '.') ? Point(j + 2, i) : Point(j - 1, i);

                        PortalInsert(portal, pos);

                        ++j; // Skip next char as it is part of the portal descriptor
                    }
                    else if (i + 1 < pMax.y && IsPortal(maze[i + 1][j]))
                    {
                        Portal portal(c, maze[i + 1][j]);
                        Point pos = (i + 2 < pMax.y && maze[i + 2][j] == '.') ? Point(j, i + 2) : Point(j, i - 1);

                        PortalInsert(portal, pos);
                    }
                }
    }
    Point target = portals.crbegin()->A;
    portals.cbegin()->B = portals.cbegin()->A;
    portals.crbegin()->B = portals.crbegin()->A;
    // Make sure Portals have A in the outside and B in the inside
    for (auto portalIter = portals.cbegin(); portalIter != portals.cend(); ++portalIter)
    {
        Point b = portalIter->B;
        if (b.y == 2 || b.x == 2 || b.y >= static_cast<int>(maze.size() - 4) || b.x >= static_cast<int>(maze[0].size() - 4))
            std::swap(portalIter->A, portalIter->B);
    }

    Bounds bounds;
    bounds += pMax + Point(-1, -1);

    // Generate connections information
    std::set<Connections> connections;
    {
        auto FindConnections = [&bounds = std::as_const(bounds), &maze = std::as_const(maze), &pointsToPortal = std::as_const(pointsToPortal),
                                &portals = std::as_const(portals), &connections](Connections& current, const Point& point)
        {
            // Fill from position, find all reachable portals through fill algorithm
            std::set<Point> processed = { point }, previousIteration = { point };
            int iterations = 0;
            std::vector<Connections> accessible;
            do
            {
                ++iterations;
                std::set<Point> currentIteration;
                for (const Point& p : previousIteration)
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        Point np = p + directions[i];
                        if (auto iter = processed.find(np);
                            iter == processed.cend() && bounds.Inside(np) && maze[np.y][np.x] == '.')
                        {
                            currentIteration.insert(np);
                            processed.insert(np);
                        }
                    }
                    if (auto iter = pointsToPortal.find(p);
                        p != point && iter != pointsToPortal.cend())
                    {
                        Portal reachedPortal = *portals.find(iter->second);

                        current.connectsTo.emplace_back(reachedPortal.name, ((reachedPortal.A == p || reachedPortal.name == exitPortal) ? Position::outer : Position::inner), iterations);
                        accessible.emplace_back(reachedPortal.name, reachedPortal.B == p && reachedPortal.name != exitPortal);
                        accessible.back().connectsTo.emplace_back(std::get<0>(current.portal), std::get<1>(current.portal), iterations);
                    }
                }
                std::swap(previousIteration, currentIteration);
            } while (!previousIteration.empty());

            // Then for each reachable portal use an A* to find distance to each other reachable portal (N^2 - N)/2 times
            for (size_t i = 0; i < accessible.size() - 1; ++i)
            {
                const Portal& startPortal = *portals.find(std::get<0>(accessible[i].portal));
                Point start = (std::get<1>(accessible[i].portal) == Position::outer ? startPortal.A : startPortal.B);
                for (size_t j = i + 1; j < accessible.size(); ++j)
                {
                    Portal targetPortal = *portals.find(std::get<0>(accessible[j].portal));
                    Point target = (std::get<1>(accessible[j].portal) == Position::outer ? targetPortal.A : targetPortal.B);

                    AStar<Point, int> aStar([target](const Point& p) { return ManhattanDistance(p, target); });
                    aStar.EstimatedEmplace(start, 1); // Factor in the cost of taking a portal
                    int cost = 0;
                    while (!aStar.Empty())
                    {
                        AStar<Point, int>::ASI currentInfo = aStar.PopFront();
                        const int nextCost = currentInfo.cost + 1;
                        for (const Point& p : directions)
                        {
                            Point np = currentInfo.info + p;
                            if (np == target)
                            {
                                cost = nextCost;
                                goto AStarCompleted; // We are done, get out
                            }
                            if (bounds.Inside(np) && !aStar.HasEvaluatedNode(np) && maze[np.y][np.x] == '.')
                                aStar.EstimatedEmplace(np, nextCost);
                        }
                    }
                AStarCompleted:
                    accessible[i].connectsTo.emplace_back(std::get<0>(accessible[j].portal), std::get<1>(accessible[j].portal), cost);
                    accessible[j].connectsTo.emplace_back(std::get<0>(accessible[i].portal), std::get<1>(accessible[i].portal), cost);
                }
            }

            // Finally, set all reachable portals as processed
            connections.insert(current);
            connections.insert(accessible.cbegin(), accessible.cend());
        };

        for (const auto& portal : portals)
        {
            if (auto current = Connections(portal.name, false); // Check if we have already processed outer portal
                connections.find(current) == connections.cend())
                FindConnections(current, portal.A);

            if (auto current = Connections(portal.name, true); // Check if we have already processed inner portal (avoid entrance and exit)
                portal.name != entrancePortal && portal.name != exitPortal && connections.find(current) == connections.cend())
                FindConnections(current, portal.B);
        }
    }

    AStar<PortalInfo, int, EvaluationPolicy::OnPop> aStar([target, &portals = std::as_const(portals)](const PortalInfo& pi) {
        const Portal& portal = *portals.find(std::get<0>(pi)); // Fetch position from PortalInfo
        return ManhattanDistance((std::get<1>(pi) == Position::outer ? portal.A : portal.B), target);
        });
    aStar.Emplace(std::make_tuple(entrancePortal, Position::inner), 0, 0);
    aStar.PopFront(); // Dirty trick to avoid processing AA as inner
    aStar.EstimatedEmplace(connections.cbegin()->portal, -1);

    while (!aStar.Empty())
    {
        if (aStar.HasEvaluatedNode(aStar.PeakFront().info))
        {
            aStar.EraseFront();
            continue;
        }
        AStarInfo<PortalInfo, int> current = aStar.PopFront();
        if (std::get<0>(current.info) == exitPortal)
        {
            std::cout << "Part 1: " << current.cost << std::endl;
            break;
        }
        for (const PortalAndDistance& pd : connections.find(current.info)->connectsTo)
            aStar.EstimatedEmplace(std::make_tuple(std::get<0>(pd), StepThrough(std::get<1>(pd))), current.cost + std::get<2>(pd));
    }

    // Going 3D for part 2 solution
    // Compute minimal cost to change layer
    int minCost = bounds.maxX - bounds.minX + bounds.maxY - bounds.minY;
    for (auto iter = portals.cbegin(); iter != portals.cend(); ++iter)
    {
        int cost = ManhattanDistance(target, iter->B);
        if (cost < minCost && cost != 0)
            minCost = cost;
    }

    using PortalInfo3D = std::tuple<std::array<char, 2>, Position, int>;
    AStar<PortalInfo3D, int, EvaluationPolicy::OnPop> aStar3D([minCost, target, &portals = std::as_const(portals)](const PortalInfo3D& pi) 
        {
            const Portal& portal = *portals.find(std::get<0>(pi));
            return ManhattanDistance(target, (std::get<1>(pi) == Position::outer ? portal.A : portal.B)) + minCost * std::get<2>(pi);
        });

    aStar3D.Emplace(std::make_tuple(entrancePortal, Position::inner, 0), 0, 0);
    aStar3D.PopFront(); // dirty trick to avoid evaluating AA inner
    aStar3D.EstimatedEmplace(std::make_tuple(entrancePortal, Position::outer, 0), -1);

    while (!aStar3D.Empty())
    {
        if (aStar3D.HasEvaluatedNode(aStar3D.PeakFront().info))
        {
            aStar3D.EraseFront();
            continue;
        }
        AStarInfo<PortalInfo3D, int> current = aStar3D.PopFront();
        if (std::get<0>(current.info) == exitPortal)
        {
            std::cout << "Part 2: " << current.cost << std::endl;
            break;
        }
        const int level = std::get<2>(current.info);
        for (const PortalAndDistance& pd : connections.find(std::make_tuple(std::get<0>(current.info), std::get<1>(current.info)))->connectsTo)
        {
            Position pos = std::get<1>(pd);
            std::array<char,2> name = std::get<0>(pd);
            if (level == 0 && pos == Position::outer && name != exitPortal)
                continue;
            if (level != 0 && (name == entrancePortal || name == exitPortal))
                continue;
            aStar3D.EstimatedEmplace(std::make_tuple(std::get<0>(pd), StepThrough(std::get<1>(pd)), level + (pos == Position::inner ? 1 : -1)), current.cost + std::get<2>(pd));
        }
    }
    return 0;
}
