#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: Day01.exe DataFilename" << std::endl;
		return -1;
	}

	std::ifstream in(argv[1], std::ios::in);

	int moduleWeight;
	int part1Result = 0;
	int part2Result = 0;
	
	while (in >> moduleWeight)
	{
		int fuel = (moduleWeight / 3) - 2;
		part1Result += fuel;
		while (fuel > 0)
		{
			part2Result += fuel;
			fuel = (fuel / 3) - 2;
		}
	}

	std::cout << "Part 1: "<< part1Result << std::endl << "Part 2: " << part2Result << std::endl;
	in.close();
	return 0;
}
