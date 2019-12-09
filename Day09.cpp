#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <array>

using i64 = long long;
using BitCode = std::map<i64, i64>;

class Program
{
	i64 GetMode(i64 offset)
	{
		switch (offset)
		{
		case 1: return (program[pc] / 100) % 10; 
		case 2:	return (program[pc] / 1000) % 10; 
		case 3: return (program[pc] / 10000) % 10;
		default: return -1;
		}
	}

	i64 ParameterValue(i64 offset)
	{	
		switch (GetMode(offset))
		{
		case 0:
			if (program.find(pc + offset) == program.cend())
				program[pc + offset] = 0;
			if (program.find(program[pc + offset]) == program.cend())
				program[program[pc + offset]] = 0;
			return program[program[pc + offset]];
		case 1:
			if (program.find(pc + offset) == program.cend())
				program[pc + offset] = 0;
			return program[pc + offset];
		case 2:
			if (program.find(pc + offset) == program.cend())
				program[pc + offset] = 0;
			if (program.find(program[pc + offset] + relative) == program.cend())
				program[program[pc + offset] + relative] = 0;
			return program[program[pc + offset] + relative];
		default:
			return 0;
		}
	}

	void WriteTo(i64 offset, i64 value)
	{
		switch (GetMode(offset))
		{
		case 0:
			if (program.find(pc + offset) == program.cend())
				program[pc + offset] = 0;
			program[program[pc + offset]] = value;
			return;
		case 2:
			if (program.find(pc + offset) == program.cend())
				program[pc + offset] = 0;
			program[program[pc + offset] + relative] = value;
			return;
		default: return;
		}
	}


public:
	void Reset(const BitCode& newProgram, bool pausable) { pc = output = relative = 0; program = newProgram; breakOnOutput = pausable; }
	bool Run(const std::vector<i64>& inputs)
	{
		int inputCount = 0;
		bool done = false, pause = false;
		while (!done && !pause)
		{
			switch (program[pc] % 100)
			{
			case 1: // +
				WriteTo( 3, ParameterValue(1) + ParameterValue(2));
				pc += 4;
				break;
			case 2: // *
				WriteTo( 3, ParameterValue(1) * ParameterValue(2));
				pc += 4;
				break;
			case 3: // =
				WriteTo(1, inputs[inputCount++]);
				pc += 2;
				break;
			case 4: // Write to output
				output = ParameterValue(1);
				std::cout << output << std::endl;
				if (breakOnOutput) pause = true;
				pc += 2;
				break;
			case 5: // Jump if true
			{
				if (ParameterValue(1) != 0)
					pc = ParameterValue(2);
				else pc += 3;
			}
			break;
			case 6: // Jump if false
				if (ParameterValue(1) == 0)
					pc = ParameterValue(2);
				else pc += 3;
				break;
			case 7: // <
				WriteTo(3, (ParameterValue(1) < ParameterValue(2)) ? 1 : 0);
				pc += 4;
				break;
			case 8: // ==
				WriteTo(3, (ParameterValue(1) == ParameterValue(2)) ? 1 : 0);
				pc += 4;
				break;
			case 9: // Relative Base +=
				relative += ParameterValue(1);
				pc += 2;
				break;
			case 99:
				done = true;
				pc += 1;
				break;
			}
		}
		return done;
	}
	inline i64 GetOutput() const { return output; }
private:
	BitCode program;
	i64 pc = 0, output = 0, relative = 0;
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

	BitCode backupProgram;

	size_t offset = 0, result = 0;
	i64 index = 0;
	do
	{
		result = data.find_first_of(',', offset);
		i64 value = (result != std::string::npos) ? atol(data.substr(offset, result - offset).c_str()) : atol(data.substr(offset).c_str());
		backupProgram[index++] = value;
		offset = result + 1;
	} while (result != std::string::npos);


	std::cout << "Part 1: ";
	Program program;
	program.Reset(backupProgram, false);
	program.Run({ 1 });

	std::cout << "Part 2: ";
	program.Reset(backupProgram, false);
	program.Run({ 2 });

	return 0;
}