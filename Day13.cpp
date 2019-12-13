#include <iostream>
#include <fstream>
#include <string>
#include "TemplatedUtilities.h"
#include <map>
#include <algorithm>

using i64 = long long;
using IntCode = std::map<i64, i64>;
using P64 = Coord<i64>;
using Bounds64 = Boundaries<i64>;
using Screen = std::map<P64, i64>;
constexpr P64 printScore(-1, 0);

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
			return program[program[pc + offset]];
		case 1:
			return program[pc + offset];
		case 2:
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
			program[program[pc + offset]] = value;
			return;
		case 2:
			program[program[pc + offset] + relative] = value;
			return;
		default: return;
		}
	}

public:
	enum class RunMode
	{
		Continuous,
		BreakOnOutput,
	} mode;

	void Reset(const IntCode& newProgram, RunMode m = RunMode::Continuous) { pc = output = relative = 0; program = newProgram; mode = m; }
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
				//std::cout << output << std::endl;
				if (mode == RunMode::BreakOnOutput) pause = true;
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
private:
	IntCode program;
	i64 pc = 0, output = 0, relative = 0;
	
};

void PrintScreen(Screen& s, const Bounds64& b)
{
	for (i64 i = b.minY; i <= b.maxY; ++i)
	{
		for (i64 j = b.minX; j <= b.maxX; ++j)
			std::cout << s[P64(j, i)];
		std::cout << std::endl;
	}
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: Day13.exe []" << std::endl;
        return -1;
    }

    std::ifstream in(argv[1], std::ios::in);
    if (!in)
        return -1;

	std::string data;
	in >> data;
	in.close();

	IntCode backupProgram;
	size_t offset = 0, result = 0;
	i64 index = 0;
	do
	{
		result = data.find_first_of(',', offset);
		backupProgram[index++] = std::stoll(result != std::string::npos ? data.substr(offset, result - offset) : data.substr(offset));
		offset = result + 1;
	} while (result != std::string::npos);

	// Part 1
	Program program;
	program.Reset(backupProgram, Program::RunMode::BreakOnOutput);
	Screen screen;
	Bounds64 bounds;
	bool completed = false;
	
	P64 paddle, ball;
	int blocks = 0;

	do
	{
		if (program.Run({}))
			break;
		i64 x = program.GetOutput();
		if (program.Run({}))
			break;
		i64 y = program.GetOutput();
		completed = program.Run({});
		P64 p(x, y);
		bounds += p;
		screen[p] = program.GetOutput();
		switch (program.GetOutput())
		{
		case 2: ++blocks; break;
		case 3:	paddle = p;	break;
		case 4: ball = p; break;
		default: break;
		}
	} while (!completed);

	PrintScreen(screen, bounds);
	std::cout << "Part 1: " << blocks << std::endl;

	backupProgram[0] = 2; // Setting to free play
	program.Reset(backupProgram, Program::RunMode::BreakOnOutput);
	screen.clear();
	completed = false;
	do
	{
		if (program.Run({ std::clamp<i64>(ball.x - paddle.x, -1, 1) }))
			break;
		i64 x = program.GetOutput();
		if (program.Run({}))
			break;
		i64 y = program.GetOutput();
		completed = program.Run({});
		P64 p(x, y);
		if (p == printScore)
			std::cout << "Score: " << program.GetOutput() << std::endl;
		screen[p] = program.GetOutput();
		switch (program.GetOutput())
		{
		case 3:	paddle = p;	break;
		case 4: ball = p; break;
		default: break;
		}
	} while (!completed);
	
    return 0;
}
