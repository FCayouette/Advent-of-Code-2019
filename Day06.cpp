#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

constexpr int hash(const char* name)
{
	int result = 0;
	while (*name)
		result = result * ('Z' - '0' + 1) + *(name++) - '0';
	return result;
}

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
	std::map<int, int> orbits;
	while (in >> input)
	{
		size_t index = input.find(')');
		int secondPart = hash(&input[index + 1]);
		input.resize(index);
		orbits[secondPart] = hash(input.c_str());
	}
	in.close();

	int part1 = 0;

	std::map<int, int>::const_iterator iter = orbits.cbegin();
	for (std::map<int, int>::const_iterator orbit = orbits.cbegin(); orbit != orbits.cend(); ++orbit)
	{
		int who = orbit->first;
		iter = orbit;
		do
		{
			++part1;
			who = iter->second;
		} while ((iter = orbits.find(who)) != orbits.cend());
	}

	std::cout << "Part 1: " << part1 << std::endl;

	std::vector<int> path;
	{
		int who = hash("YOU");
		while ((iter = orbits.find(who)) != orbits.cend())
		{
			who = iter->second;
			path.push_back(who);
		}
	}

	int part2 = 0;
	int who = hash("SAN");
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