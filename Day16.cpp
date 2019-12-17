#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>

using i64 = long long;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: Day16.exe [filename]" << std::endl;
		return -1;
	}

	std::ifstream in(argv[1], std::ios::in);
	if (!in)
		return -1;

	std::string largenum;
	in >> largenum;
	in.close();

	std::vector<int> inputSignal, workSignal;
	inputSignal.reserve(largenum.size());
	workSignal.resize(largenum.size());
	for (char c : largenum)
		inputSignal.push_back(c - '0');

	for (int iteration = 0; iteration < 100; ++iteration)
	{
		for (int i = 0; i < inputSignal.size(); ++i)
		{
			i64 digit = 0;
			bool add = true;
			for (int j = i; j < inputSignal.size();)
			{
				for (int k = 0; k < i + 1 && j < inputSignal.size(); ++k)
					digit += (add ? inputSignal[j++] : -inputSignal[j++]);
				add = !add;
				j += i + 1; // skipping sequence of zeros
			}
			workSignal[i] = std::abs(digit) % 10;
		}
		std::swap(workSignal, inputSignal);
	}
	std::cout << "Part 1: ";
	for (int i = 0; i < 8; ++i)
		std::cout << inputSignal[i];
	std::cout << std::endl << "Part 2: ";
	
	inputSignal.clear();
	i64 index = 0;
	for (char c : largenum)
		workSignal[index++] = c - '0';
	for (int i = 0; i < 10000; ++i)
		inputSignal.insert(inputSignal.end(), workSignal.cbegin(), workSignal.cend());

	i64 offset = std::stoll(largenum.substr(0, 7));
	
	for (int iteration = 0; iteration < 100; ++iteration)
	{
		i64 partialSum = std::accumulate(inputSignal.cbegin() + offset, inputSignal.cend(), 0ll);
		for (int i = offset; i < inputSignal.size(); ++i)
		{
			i64 val = partialSum;
			partialSum -= inputSignal[i];
			inputSignal[i] = (val < 0 ? -val : val) % 10;
		}
	}

	for (int i = offset; i < offset + 8; ++i)
		std::cout << inputSignal[i];
	std::cout << std::endl;
	return 0;
}