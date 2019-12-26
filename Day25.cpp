#include <iostream>
#include <fstream>
#include <string>
#include "TemplatedUtilities.h"
#include <map>
#include <vector>
#include <deque>

using i64 = long long;
using IntCode = std::map<i64, i64>;

class Program
{

public:
	enum class RunMode
	{
		Verbose,
		Silent,
		BreakOnOutput,
	};
	enum class ReturnState
	{
		WaitingOnInput,
		WaitingOnOutput,
		Stopped
	};

	Program(const IntCode& newProgram, RunMode m = RunMode::Verbose) { Reset(newProgram, m); }
	Program(const std::string& s, RunMode m = RunMode::Verbose) : pc(0), output(0), relative(0), mode(m) { ConvertToIntCode(s, program); }
	void Reset(const IntCode& newProgram, RunMode m = RunMode::Verbose) { pc = output = relative = 0; program = newProgram; mode = m; }
	ReturnState ResetAndRun(const IntCode& newProgram, const std::vector<i64>& inputs) { pc = output = relative = 0; program = newProgram; return Run(inputs); }
	ReturnState Run(const std::vector<i64>& inputs)
	{
		inputCount = 0;
		bool done = false;
		waitingForInput = false;
		while (!done)
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
				if (inputs.size() != inputCount)
				{
					WriteTo(1, inputs[inputCount++]);
					pc += 2;
				}
				else
				{
					waitingForInput = true;
					return ReturnState::WaitingOnInput;
				}
				break;
			case 4: // Write to output
				output = ParameterValue(1);
				if (mode == RunMode::Verbose)
				{
					if (output < 256)
						std::cout << (char)output;
					else
						std::cout << output << std::endl;
				}
				pc += 2;
				if (mode == RunMode::BreakOnOutput) return ReturnState::WaitingOnOutput;
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
		return ReturnState::Stopped;
	}
	inline i64 GetOutput() const { return output; }
	//inline const IntCode& GetIntCode() const { return program; }
	inline bool WaitingForInput() const { return waitingForInput; }
	inline int GetInputCount() const { return inputCount; }
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
	bool waitingForInput;
	int inputCount = 0;
};

std::vector<i64> TextToInput(const std::string& text)
{
	std::vector<i64> result;
	for (char c : text)
		result.push_back((i64)c);
	result.push_back(10);
	return result;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: Day25.exe [filename]" << std::endl;
		return -1;
	}

	std::ifstream in(argv[1], std::ios::in);
	if (!in)
		return -1;
	std::string data;
	in >> data;
	in.close();

	Program p(data, Program::RunMode::Verbose);
	p.Run({}); // Run until we require an input

	std::string s;
	while (std::getline(std::cin, s)) // Play the game in the console
	{
		if (p.Run( TextToInput(s) ) == Program::ReturnState::Stopped)
			break;
	}
	return 0;
}

