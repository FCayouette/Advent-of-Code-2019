#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: day06.exe [filename]" << std::endl;
        return -1;
    }
    std::ifstream in(argv[1], std::ios::in);
    if (!in)
        return -1;

    std::string input;
    std::map<std::string, std::string> orbits;
    while (in >> input)
    {
        std::string first, second;
        size_t index = input.find(')');
        orbits[input.substr(index + 1)] = input.substr(0, index);
    }
    in.close();

    int part1 = 0;

    std::map<std::string, std::string>::const_iterator iter;
    for (const auto& orbit : orbits)
    {
        std::string who = orbit.first;
        while ((iter = orbits.find(who)) != orbits.cend())
        {
            ++part1;
            who = iter->second;
        }
    }

    std::cout  << "Part 1: " << part1 << std::endl;

    std::vector<std::string> path;
    {
        std::string who = "YOU";
        while ((iter = orbits.find(who)) != orbits.cend())
        {
            who = iter->second;
            path.push_back(who);
        }
    }

    int part2 = 0;
    std::string who = "SAN";
    while ((iter = orbits.find(who)) != orbits.cend())
    {
        who = iter->second;
        auto pathIter = std::find(path.cbegin(), path.cend(), who);
        if (pathIter != path.end())
        {
            part2 += pathIter - path.cbegin();
            break;
        }
        ++part2;
    }
    
    std::cout << "Part 2: " << part2 << std::endl;
    return 0;
}
