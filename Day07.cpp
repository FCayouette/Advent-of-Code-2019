#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <array>

bool RunProgram(std::vector<int>& program, const std::vector<int>& inputs, int& output, int& pc, bool pauseOnOutput = false)
{
	int inputCount = 0;
	bool done = false, pause = false;
	while (!done && !pause)
	{
		switch (program[pc] % 100)
		{
		case 1: // +
			program[program[pc + 3]] = ((program[pc] / 100) % 10 == 1 ? program[pc + 1] : program[program[pc + 1]]) + ((program[pc] / 1000) % 10 == 1 ? program[pc + 2] : program[program[pc + 2]]);
			pc += 4;
			break;
		case 2: // *
			program[program[pc + 3]] = ((program[pc] / 100) % 10 == 1 ? program[pc + 1] : program[program[pc + 1]]) * ((program[pc] / 1000) % 10 == 1 ? program[pc + 2] : program[program[pc + 2]]);
			pc += 4;
			break;
		case 3: // =
			program[program[pc + 1]] = inputs[inputCount++];
			pc += 2;
			break;
		case 4: // Print
			output = ((program[pc] / 100) % 10 == 1 ? program[pc + 1] : program[program[pc + 1]]);
			if (pauseOnOutput) pause = true;
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
	return done;
}

struct Program
{
	std::vector<int> p;
	int pc = 0, output = 0;
	bool breakOnOutput = false;

	void Reset(const std::vector<int>& newProgram, bool pausable) { pc = output = 0; p = newProgram; breakOnOutput = pausable; }
	bool Run(const std::vector<int>& inputs) {	return RunProgram(p, inputs, output, pc, breakOnOutput); }
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: Day07.exe [filename]" << std::endl;
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

	int maxOutput = 0;
	std::vector<int> combinations = { 0, 1, 2, 3, 4 };
	std::vector<int> bestPermutation;

	do
	{
		int output = 0, pc = 0;
		RunProgram(program, { combinations[0], 0 }, output, pc = 0);
		program = backup;
		RunProgram(program, { combinations[1], output }, output, pc = 0);
		program = backup;
		RunProgram(program, { combinations[2], output }, output, pc = 0);
		program = backup;
		RunProgram(program, { combinations[3], output }, output, pc = 0);
		program = backup;
		RunProgram(program, { combinations[4], output }, output, pc = 0);
		program = backup;

		if (output > maxOutput)
		{
			bestPermutation = combinations;
			maxOutput = output;
		}
	} while (std::next_permutation(combinations.begin(), combinations.end()));
	
	std::cout << "Part 1: " << maxOutput << std::endl;


	// Part 2;
	combinations = { 5, 6, 7, 8, 9 };

	maxOutput = 0;
	std::array<Program, 5> programs;
	
	do {
		for (Program& p : programs)
			p.Reset(backup, true);
		for (int i = 0; i < 5; ++i)
			programs[i].Run({ combinations[i], programs[(i + 4) % 5].output });

		int i = 0;
		bool completed = false;
		while (!completed)
		{
			completed = programs[i].Run({ programs[(i + 4) % 5].output }) && i == 4;
			i = (i + 1) % 5;
		}
		if (programs[4].output > maxOutput)
		{
			bestPermutation = combinations;
			maxOutput = programs[4].output;
		}
	} while (std::next_permutation(combinations.begin(), combinations.end()));
	std::cout << "Part 2: " << maxOutput << std::endl;
	
    return 0;
}
