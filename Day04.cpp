#include <iostream>
#include <fstream>
#include <string>

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
		int value[6] = {i/100000, (i%100000)/10000, (i%10000)/1000, (i%1000)/100, (i%100)/10, i%10};
		bool adjacent = false, monotonic = true;
		int histogram[10] = { 0,0,0,0,0,0,0,0,0,0 };
		for (int j = 0; j < 6; ++j)
		{
			++histogram[value[j]];
			if (j < 5)
			{
				if (value[j] == value[j + 1])
					adjacent = true;
				else if (value[j] > value[j + 1])
				{
					monotonic = false;
					break;
				}
			}
		}
		
		if (adjacent && monotonic)
		{
			++part1;
			if (std::find(histogram, histogram + 10, 2) != histogram + 10)
				++part2;
		}
	}

	std::cout << "Part 1: " << part1 << std::endl << "Part 2: " << part2 << std::endl;
	return 0;
}
