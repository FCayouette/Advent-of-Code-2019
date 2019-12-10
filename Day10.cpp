#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <set>

struct Asteroid
{
    int x, y;
    float angle;
    constexpr Asteroid(int X = 0, int Y = 0, float a = 0.f) : x(X), y(Y), angle(a) {}
    constexpr bool operator <(const Asteroid& p) const { return angle < p.angle; }
};

float computeAngle(int dY, int dX)
{
    float result;
    if (dY < 0 && dX == 0) // Would normally return +PI, but we need it to be the small instead
        result = -4.f; 
    else 
        result = std::atan2f((float)-dX, (float)dY);

    return result;
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: Day10.exe []" << std::endl;
        return -1;
    }
    std::ifstream in(argv[1], std::ios::in);
    if (!in)
        return -1;

    std::string input;
    std::vector<std::string> field;
    while (in >> input)
        field.emplace_back(std::move(input));

    in.close();

    const int X = field.front().size();
    const int Y = field.size();

    int maxCount = 0, bestX = -1, bestY = -1;
    std::set<Asteroid> bestAsteroids;
    for (int y = 0; y < Y; ++y)
        for (int x = 0; x < X; ++x)
            if (field[y][x] == '#')
            {
                int seeCount = 0;
                std::set<Asteroid> asteroids;
                
                for (int i = 0; i < Y; ++i)
                    for (int j = 0; j < X; ++j)
                    {
                        if (j == x && i == y)
                            continue;
                        if (field[i][j] == '#')
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
                                check = false;
                                insert = true;
                                int common = std::gcd(dX, dY); //C++17
                                while (common > 1)
                                {
                                    dX /= common;
                                    dY /= common;
                                    common = std::gcd(dX, dY); //C++17
                                    check = true;
                                }
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
                                asteroids.emplace(j, i, computeAngle(dY, dX));
                            }
                        }
                    }

                if (seeCount > maxCount)
                {
                    maxCount = seeCount;
                    bestX = x;
                    bestY = y;
                    std::swap(bestAsteroids, asteroids);
                }
            }
    
    std::cout << "Best count " << maxCount << " @ " << bestX << ',' << bestY << std::endl;

    if (bestAsteroids.size() >= 200)
    {
        auto iter = bestAsteroids.cbegin();
        for (int i = 0; i < 199; ++i)
            ++iter;
        std::cout << "Part 2: " << iter->x * 100 + iter->y << std::endl;
    }
    else
        std::cout << "Data set more complex that one received, sorry" << std::endl;
    
    return 0;
}