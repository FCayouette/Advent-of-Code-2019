#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>

namespace
{
    static int X;
    static int Y;
}

using Field = std::vector<std::string>;

struct Asteroid
{
    int x, y;
    float angle;
    constexpr Asteroid(int X = 0, int Y = 0, float a = 10.f) : x(X), y(Y), angle(a) {}
    constexpr bool operator <(const Asteroid& p) const { return angle < p.angle; }
};

float computeAngle(int dX, int dY)
{
    float result;
    if (dY < 0 && dX == 0) // Would normally return +PI, but we need it to be negative PI or less instead
        result = -4.f; 
    else 
        result = std::atan2f((float)-dX, (float)dY);

    return result;
}

int count(const Field& field, int x, int y, std::vector<Asteroid>& asteroids)
{
    int seeCount = 0;
    for (int i = 0; i < Y; ++i)
        for (int j = 0; j < X; ++j)
            if (field[i][j] == '#' && !(j == x && i == y))
            {
                int dX = j - x;
                int dY = i - y;
                bool check = true, insert = false;
                if (dX == 0)
                    dY = (dY < 0 ? -1 : 1);
                else if (dY == 0)
                    dX = (dX < 0 ? -1 : 1);
                else
                {
                    insert = true;
                    if (int common = std::gcd(dX, dY); //C++17
                        common > 1)
                    {
                        dX /= common;
                        dY /= common;
                        check = true;
                    }
                    else check = false;
                }
                if (check)
                {
                    int mul = 1;
                    while (field[y + mul * dY][x + mul * dX] != '#')
                        ++mul;
                    insert = (y + mul * dY == i && x + mul * dX == j);
                }
                if (insert)
                {
                    ++seeCount;
                    asteroids.emplace_back(j, i, computeAngle(dX, dY));
                }
            }
    return seeCount;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: Day10.exe [filename]" << std::endl;
        return -1;
    }
    std::ifstream in(argv[1], std::ios::in);
    if (!in)
        return -1;

    std::string input;
    Field field;
    while (in >> input)
        field.emplace_back(std::move(input));

    in.close();

    X = field.front().size();
    Y = field.size();

    int maxCount = 0, bestX = -1, bestY = -1;
    std::vector<Asteroid> bestAsteroids;
    for (int y = 0; y < Y; ++y)
        for (int x = 0; x < X; ++x)
            if (field[y][x] == '#')
            {
                std::vector<Asteroid> asteroids;
                asteroids.reserve(X * Y);

                int seeCount = count(field, x, y, asteroids);

                if (seeCount > maxCount)
                {
                    maxCount = seeCount;
                    bestX = x;
                    bestY = y;
                    std::swap(bestAsteroids, asteroids);
                }
            }

    std::cout << "Best count " << maxCount << " @ " << bestX << ',' << bestY << std::endl;

    size_t target = 200;
    while (bestAsteroids.size() < target)
    {
        for (const Asteroid& a : bestAsteroids)
            field[a.y][a.x] = '~';
        target -= bestAsteroids.size();
        bestAsteroids.clear();
        count(field, bestX, bestY, bestAsteroids);
    }
    std::sort(bestAsteroids.begin(), bestAsteroids.end());
    --target;
    std::cout << "Part 2: " << bestAsteroids[target].x * 100 + bestAsteroids[target].y << std::endl;

    return 0;
}