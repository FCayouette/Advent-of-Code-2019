#include <iostream>
#include <fstream>
#include <string>
#include <vector>

void RunProgram(std::vector<int>& program, int input)
{
	int pc = 0;
	bool done = false;
	while (!done)
	{

		switch (program[pc]%100)
		{
		case 1: // +
			program[program[pc + 3]] = ((program[pc]/100)%10 == 1? program[pc+1] : program[program[pc+1]]) + ((program[pc] / 1000) % 10 == 1 ? program[pc + 2] : program[program[pc + 2]]);
			pc += 4;
			break;
		case 2: // *
			program[program[pc + 3]] = ((program[pc] / 100) % 10 == 1 ? program[pc + 1] : program[program[pc + 1]]) * ((program[pc] / 1000) % 10 == 1 ? program[pc + 2] : program[program[pc + 2]]);
			pc += 4;
			break;
		case 3: // =
			program[program[pc + 1]] = input;
			pc += 2;
			break;
		case 4: // Print
			std::cout << ((program[pc] / 100) % 10 == 1 ? program[pc + 1] : program[program[pc + 1]]) << std::endl;
			pc += 2;
			break;
		case 5: // Jump if true
		{
			if (((program[pc] / 100) % 10 == 1 ? program[pc + 1] : program[program[pc + 1]]) != 0)
				pc = ((program[pc] / 1000) % 10 == 1 ? program[pc + 2] : program[program[pc + 2]]);
			else pc += 3;
		}
			break;
		case 6: // Jump if false
			if (((program[pc] / 100) % 10 == 1 ? program[pc + 1] : program[program[pc + 1]]) == 0)
				pc = ((program[pc] / 1000) % 10 == 1 ? program[pc + 2] : program[program[pc + 2]]);
			else pc += 3;
			break;
		case 7: // <
			program[program[pc + 3]] = (((program[pc] / 100) % 10 == 1 ? program[pc + 1] : program[program[pc + 1]]) < ((program[pc] / 1000) % 10 == 1 ? program[pc + 2] : program[program[pc + 2]])) ? 1 : 0;
			pc += 4;
			break;
		case 8: // ==
			program[program[pc + 3]] = (((program[pc] / 100) % 10 == 1 ? program[pc + 1] : program[program[pc + 1]]) == ((program[pc] / 1000) % 10 == 1 ? program[pc + 2] : program[program[pc + 2]])) ? 1 : 0;
			pc += 4;
			break;
		case 99:
			done = true;
			pc += 1;
			break;
		}
	}
}


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: Day05.exe [ProgramFilename]" << std::endl;
		return -1;
	}
	std::ifstream in(argv[1], std::ios::in);
	if (!in)
		return -1;


	std::string data;
	in >> data;
	in.close();

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

	// Part 1
	RunProgram(program, 1);

	// Part 2
	RunProgram(backup, 5);

	return 0;
}