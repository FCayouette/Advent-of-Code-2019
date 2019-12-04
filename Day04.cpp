#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: Day04.exe range" << std::endl;
		return -1;
	}

	// Range given as input was 235741-706948

	// Process range string
	std::string input = argv[1];
	size_t dash = input.find_first_of('-');
	int minRange = atoi(input.substr(0, dash).c_str());
	int maxRange = atoi(input.substr(dash + 1).c_str());

	int part1 = 0, part2 = 0;
	for (int i = minRange; i <= maxRange; ++i)
	{
		int value[6] = { i / 100000, (i % 100000) / 10000, (i % 10000) / 1000, (i % 1000) / 100, (i % 100) / 10, i % 10 };
		bool monotonic = true;
		int histogram[10] = { 0,0,0,0,0,0,0,0,0,0 };
		for (int j = 0; j < 6; ++j)
		{
			++histogram[value[j]];
			if (j < 5 && value[j] > value[j + 1])
			{
				monotonic = false;
				// Optim: Jump to the next monotonic value
				for (int k = j + 1; k < 6; ++k)
					value[k] = value[j];
				i = 0;
				for (int k = 0; k < 6; ++k)
					i = i * 10 + value[k];
				--i; // Prepare for next interation
				break;
			}
		}

		if (monotonic && std::find_if(histogram, histogram + 10, [](int val) { return val >= 2; }) != histogram + 10 )
		{
			++part1;
			if (std::find(histogram, histogram + 10, 2) != histogram + 10)
				++part2;
		}
	}
	std::cout << "Part 1: " << part1 << std::endl << "Part 2: " << part2 << std::endl;
	return 0;
}