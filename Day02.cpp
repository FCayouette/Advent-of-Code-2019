#include <iostream>
#include <fstream>
#include <string>
#include <vector>

void RunProgram(std::vector<int>& program)
{
	int pc = 0;
	while (program[pc] != 99)
	{
		if (program[pc] == 1)
		{
			program[program[pc + 3]] = program[program[pc + 1]] + program[program[pc + 2]];
			pc += 4;
		}
		else if (program[pc] == 2)
		{
			program[program[pc + 3]] = program[program[pc + 1]] * program[program[pc + 2]];
			pc += 4;
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
	std::cout << program[0] << std::endl;

	// Part 2
	int noun = 1, verb = 1;
	constexpr int target = 19690720;
	while (true)
	{
		program = backup;
		program[1] = noun;
		program[2] = verb;
		RunProgram(program);

		if (program[0] == target)
			break;
		
		if (program[0] < target)
			++noun;
		else
		{
			++verb;
			noun = 1;
		}
	}
	std::cout << std::endl << 100 * noun + verb << " " << noun << " " << verb << std::endl;

	return 0;
}
