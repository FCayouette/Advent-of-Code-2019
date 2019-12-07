#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <array>

class Program
{
public:
	void Reset(const std::vector<int>& newProgram, bool pausable) { pc = output = 0; program = newProgram; breakOnOutput = pausable; }
	bool Run(const std::vector<int>& inputs)
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
			case 4: // Write to output
				output = ((program[pc] / 100) % 10 == 1 ? program[pc + 1] : program[program[pc + 1]]);
				if (breakOnOutput) pause = true;
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
	inline int GetOutput() const { return output; }
private:
	std::vector<int> program;
	int pc = 0, output = 0;
	bool breakOnOutput = false;

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
	std::vector<int> backupProgram;
	do
	{
		result = data.find_first_of(',', offset);
		int value = (result != std::string::npos) ? atoi(data.substr(offset, result - offset).c_str()) : atoi(data.substr(offset).c_str());
		backupProgram.push_back(value);
		offset = result + 1;
	} while (result != std::string::npos);

	std::array<Program, 5> programs;
	
	int maxOutput = 0;
	std::vector<int> combinations = { 0, 1, 2, 3, 4 };
	
	do
	{
		for (Program& p : programs)
			p.Reset(backupProgram, false);
		for (int i = 0; i < 5; ++i)
			programs[i].Run({ combinations[i], programs[(i + 4) % 5].GetOutput() });

		maxOutput = std::max(maxOutput, programs[4].GetOutput());
	} while (std::next_permutation(combinations.begin(), combinations.end()));
	
	std::cout << "Part 1: " << maxOutput << std::endl;

	// Part 2;
	combinations = { 5, 6, 7, 8, 9 };
	maxOutput = 0;
	do {
		for (Program& p : programs)
			p.Reset(backupProgram, true);
		// First iteration through the feedback look, requires combination # as first input
		for (int i = 0; i < 5; ++i)
			programs[i].Run({ combinations[i], programs[(i + 4) % 5].GetOutput() });

		int i = 0;
		bool completed = false;
		while (!completed)
		{
			completed = programs[i].Run({ programs[(i + 4) % 5].GetOutput() }) && i == 4;
			i = (i + 1) % 5;
		}

		maxOutput = std::max(maxOutput, programs[4].GetOutput());
	} while (std::next_permutation(combinations.begin(), combinations.end()));

	std::cout << "Part 2: " << maxOutput << std::endl;
	
    return 0;
}
