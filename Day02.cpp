#include <iostream>
#include <fstream>
#include <string>
#include <vector>

void RunProgram(std::vector<int>& program)
{
	int pc = 0;
	while (program[pc] != 99)
	{
		switch (program[pc])
		{
		case 1:
			program[program[pc + 3]] = program[program[pc + 1]] + program[program[pc + 2]];
			pc += 4;
			break;
		case 2:
			program[program[pc + 3]] = program[program[pc + 1]] * program[program[pc + 2]];
			pc += 4;
			break;
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage Day02.exe [filename]" << std::endl;
		return -1;
	}

	std::ifstream in(argv[1], std::ios::in);
	if (!in)
		return -1;

	std::string data;
	in >> data;
	
	size_t offset = 0, result = 0;
	std::vector<int> program, backup;
	do
	{
		result = data.find_first_of(',', offset);
		int value = (result != std::string::npos) ? atoi(data.substr(offset, result - offset).c_str()) : atoi(data.substr(offset).c_str());
		program.push_back(value);
		offset = result + 1;
	} while (result != std::string::npos);

	backup = program;

	// Part 1;
	program[1] = 12;
	program[2] = 2;
	RunProgram(program);
	std::cout << "Part 1: " << program[0] << std::endl;

	// Part 2, performing binary search through the possible noun values and computing verb according to best result
	int low = 0, high = program.size(), verb = 0, highestResultBelowTarget = 0, bestNoun = 0;
	int noun = high / 2;
	constexpr int target = 19690720;
	while (true)
	{
		program = backup;
		program[1] = noun;
		program[2] = verb;
		RunProgram(program);
		
		if (program[0] == target)
			break; // In case verb is 0
		
		if (program[0] < target)
		{
			low = noun;
			if (highestResultBelowTarget < program[0])
			{
				highestResultBelowTarget = program[0];
				bestNoun = noun;
			}
		}
		else
			high = noun;

		int next = (high + low) / 2;
		if (next == noun)
		{  // We have found the best noun, verb is simply the difference to the target.
			noun = bestNoun;
			verb = target - highestResultBelowTarget;
			break;
		}
		else
			noun = next;
	}
	std::cout << "Part 2: " << 100 * noun + verb << std::endl;

	return 0;
}
