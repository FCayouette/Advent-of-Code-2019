#include <iostream>
#include <fstream>
#include <string>
#include "TemplatedUtilities.h"
#include <map>

using i64 = long long;
using IntCode = std::map<i64, i64>;
using P64 = Coord<i64>;

class Program
{

public:
	enum class RunMode
	{
		Verbose,
		Silent,
		BreakOnOutput,
	};

	Program(const IntCode& newProgram, RunMode m = RunMode::Verbose) { Reset(newProgram, m); }
	Program(const std::string& s, RunMode m = RunMode::Verbose) : pc(0), output(0), relative(0), mode(m) { ConvertToIntCode(s, program); }
	void Reset(const IntCode& newProgram, RunMode m = RunMode::Verbose) { pc = output = relative = 0; program = newProgram; mode = m; }
	bool ResetAndRun(const IntCode& newProgram, const std::vector<i64>& inputs) { pc = output = relative = 0; program = newProgram; return Run(inputs); }
	bool Run(const std::vector<i64>& inputs)
	{
		int inputCount = 0;
		bool done = false, pause = false;
		while (!done && !pause)
		{
			switch (program[pc] % 100)
			{
			case 1: // +
				WriteTo(3, ParameterValue(1) + ParameterValue(2));
				pc += 4;
				break;
			case 2: // *
				WriteTo(3, ParameterValue(1) * ParameterValue(2));
				pc += 4;
				break;
			case 3: // =
				WriteTo(1, inputs[inputCount++]);
				pc += 2;
				break;
			case 4: // Write to output
				output = ParameterValue(1);
				if (mode == RunMode::BreakOnOutput) pause = true;
				else if (mode == RunMode::Verbose)
					std::cout << output << std::endl;
				pc += 2;
				break;
			case 5: // Jump if true
				if (ParameterValue(1) != 0)
					pc = ParameterValue(2);
				else pc += 3;
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
	inline const IntCode& GetIntCode() const { return program; }

	static void ConvertToIntCode(const std::string& s, IntCode& target)
	{
		target.clear();
		size_t offset = 0, result = 0;
		i64 index = 0;
		do
		{
			result = s.find_first_of(',', offset);
			target[index++] = std::stoll(result != std::string::npos ? s.substr(offset, result - offset) : s.substr(offset));
			offset = result + 1;
		} while (result != std::string::npos);
	}
private:
	// Utility methods
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
		case 0: return program[program[pc + offset]];
		case 1: return program[pc + offset];
		case 2:	return program[program[pc + offset] + relative];
		default: return 0;
		}
	}

	void WriteTo(i64 offset, i64 value)
	{
		switch (GetMode(offset))
		{
		case 0:	program[program[pc + offset]] = value; return;
		case 2:	program[program[pc + offset] + relative] = value; return;
		default: return;
		}
	}

	IntCode program;
	i64 pc = 0, output = 0, relative = 0;
	RunMode mode;
};

std::vector<i64> SpringCodeToInput(const std::vector<std::string>& program )
{
	std::vector<i64> result;
	for (const std::string& s : program)
	{
		for (char c : s)
			result.push_back((i64)c);
		result.push_back(10);
	}
	return result;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: Day21.exe [filename]" << std::endl;
		return -1;
	}

	std::ifstream in(argv[1], std::ios::in);
	if (!in)
		return-1;
	std::string data;
	in >> data;
	in.close();
	IntCode backup;
	Program::ConvertToIntCode(data, backup);

	Program program(backup, Program::RunMode::BreakOnOutput);

	std::vector<std::string> springCode =
	{ "NOT C J", // jump if there is hole 3 away
	  "AND D J", // and we can stick the landing
	  "NOT A T", // or just save our skin to 
	  "OR T J",  // avoid falling in the hole next to us
	  "WALK" };

	bool done = false;
	auto programInput = SpringCodeToInput(springCode);
	do
	{
		done = program.Run(programInput);
		if (i64 output = program.GetOutput();
			output >= 256)
		{
			std::cout << "Part 1: " << output << std::endl;
			done = true;
		}
		else std::cout << (char)program.GetOutput();
	} while (!done);

	// Part 2
	springCode =
	{ "OR A J",
	  "AND B J", // Jump if there is a hole in the next 3
	  "AND C J",
	  "NOT J J",
	  "AND D J", // But only if we can land 
	  "OR E T",  // And we either can run
	  "OR H T",  // or jump right after
	  "AND T J",
	  "RUN" };

	done = false;
	program.Reset(backup, Program::RunMode::BreakOnOutput);
	programInput = SpringCodeToInput(springCode);
	do
	{
		done = program.Run(programInput);
		if (i64 output = program.GetOutput();
			output >= 256)
		{
			std::cout << "Part 2: " << output << std::endl;
			done = true;
		}
		else std::cout << (char)program.GetOutput();
	} while (!done);
	return 0;
}