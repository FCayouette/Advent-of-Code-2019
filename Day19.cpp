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

	IntCode program;
	i64 pc = 0, output = 0, relative = 0;
	RunMode mode;
};

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: Day18.exe [filename]" << std::endl;
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
	
	int count = 0;
	int lastlineCount = 0;
	P64 lastPoint;
	Program program(backup, Program::RunMode::BreakOnOutput);
	for (i64 i = 0; i < 50; ++i)
	{
		for (i64 j = 0; j < 50; ++j)
		{
			program.Run({ j, i });
			if (program.GetOutput() == 1)
			{
				++count;
				lastPoint = P64(j, i);
			}
			std::cout << (program.GetOutput() == 1) ? '#' : '.';
			program.Reset(backup, Program::RunMode::BreakOnOutput);
		}
		std::cout << std::endl;
	}
	std::cout << "PArt 1: " << count << " " << lastPoint.x << ',' << lastPoint.y << std::endl;

	// Y axis is limiting factor
	// Go in steps until Y is of height 100
	
	int yCount = 0;
	while (yCount <= 100)
	{
		do
		{
			lastPoint += P64(1, 0);
			program.Reset(backup, Program::RunMode::BreakOnOutput);
			program.Run({ lastPoint.x, lastPoint.y });
		} while (program.GetOutput() == 1);
		lastPoint += P64(-1, 0);
		yCount = 0;
		do {
			lastPoint += P64(0, 1);
			program.Reset(backup, Program::RunMode::BreakOnOutput);
			program.Run({ lastPoint.x, lastPoint.y });
			++yCount;
		} while (program.GetOutput() == 1);
		lastPoint += P64(0, -1);
	}

	// Now that we have a starting point, find a point xy such as 
	// P(X,Y), P(X+99, Y) and P(X,Y+99)== 1 and 
	// P(X+100,Y) and (P(X, Y+100) == 0

	P64 point = lastPoint + P64(99,0);
	do
	{
		program.Reset(backup, Program::RunMode::BreakOnOutput);
		point += P64(1, 0);
		program.Run({ point.x, point.y });
		
	} while (program.GetOutput() == 1);

	point += P64(-100, 0);
	
	P64 adjust;
	constexpr P64 origin(0, 0);
	do
	{
		adjust = origin;
		int x = 0;
		int y = 0;
		program.Reset(backup, Program::RunMode::BreakOnOutput);
		program.Run({ point.x+99, point.y });
		if (program.GetOutput() == 1)
		{
			program.Reset(backup, Program::RunMode::BreakOnOutput);
			program.Run({ point.x + 100, point.y });
			x += program.GetOutput() + 1;
		}
		if (x == 1)
		{
			program.Reset(backup, Program::RunMode::BreakOnOutput);
			program.Run({ point.x, point.y + 99 });
			if (program.GetOutput() == 1)
			{	
				program.Reset(backup, Program::RunMode::BreakOnOutput);
				program.Run({ point.x, point.y + 100 });
				y += program.GetOutput() + 1;
			}
		} 
		else if (x == 0) adjust.y = 1;
		else adjust.y =-1;
		if (y == 0) adjust.x = 1;
		else if (y == 2) adjust.x = -1;
		point += adjust;
	} while (adjust != origin);

	std::cout << "Part 2: " << point.x * 10000 + point.y << std::endl;
	return 0;
}
