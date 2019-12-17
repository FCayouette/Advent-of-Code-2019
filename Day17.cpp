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
		Verbose,
		Silent,
		BreakOnOutput,
	};

	Program(const IntCode& newProgram, RunMode m = RunMode::Verbose) { Reset(newProgram, m); }
	Program(const std::string& s, RunMode m = RunMode::Verbose) : pc(0), output(0), relative(0), mode(m) { ConvertToIntCode(s, program); }
	void Reset(const IntCode& newProgram, RunMode m = RunMode::Verbose) { pc = output = relative = 0; program = newProgram; mode = m; }
	void Restart() { pc = output = relative = 0; }
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
	IntCode program;
	i64 pc = 0, output = 0, relative = 0;
	RunMode mode;
};

bool IsScaffold(std::map<Point, char>& screen, const Bounds& bounds, const Point& p)
{
	return bounds.Inside(p) && screen[p] == '#';
}

std::vector<i64> CommandToInput(const std::string& s)
{
	std::vector<i64> result;
	result.reserve(s.size() + 1);
	for (char c : s)
		result.push_back((i64)c);
	result.push_back(10);
	return result;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: Day17.exe [filename]" << std::endl;
        return -1;
    }

    std::ifstream in(argv[1], std::ios::in);
    if (!in)
        return -1;
    
	std::string data;
	in >> data;
	in.close();
	Program program(data, Program::RunMode::BreakOnOutput);
	IntCode backup = program.GetIntCode();
	bool completed = false;
	std::map<Point, char> screen;
	Point pixel(0, 0);
	Bounds bounds;
	Point botPos;
	do {
		completed = program.Run({});
		if (!completed)
		{
			i64 output = program.GetOutput();
			switch (output)
			{
			case 10:
				pixel = Point(0, pixel.y + 1);
				std::cout << std::endl;
				break;
			default:
			{
				char c = static_cast<char>(output);
				std::cout << c;
				screen[pixel] = c;
				bounds += pixel;

				if (c == '^')
					botPos = pixel;

				pixel = Point(pixel.x + 1, pixel.y);
			}
				break;
			}
		}
		
	} while (!completed);

	int sum = 0;
	for (int y = bounds.minY + 1; y < bounds.maxY; ++y)
		for (int x = bounds.minX + 1; x < bounds.maxX; ++x)
		{
			Point pos = Point(x, y);
			if (screen[pos] == '#')
			{
				int count = 0;
				for (int i = 0; i < 4; ++i)
				{
					if (screen[pos + directions[i]] == '#')
						++count;
					else break;
				}
				if (count == 4)
					sum += x * y;
			}
		}

	std::cout << "Part 1: " << sum << std::endl;

	// First extract the full command line
	std::string fullCommand;
	int botDirection = 3;
	
	bool canTurn = true;
	while (canTurn)
	{
		if (IsScaffold(screen, bounds, botPos + directions[(botDirection + 1) % 4]))
		{
			fullCommand += "R,";
			botDirection = (botDirection + 1) % 4;
		}
		else if (IsScaffold(screen, bounds, botPos + directions[(botDirection + 3) % 4]))
		{
			fullCommand += "L,";
			botDirection = (botDirection + 3) % 4;
		}
		else canTurn = false;
		if (canTurn)
		{
			// Find the extent of the move
			int count = 0;
			while (bounds.Inside(botPos + directions[botDirection]) && screen[botPos + directions[botDirection]] == '#')
			{
				botPos += directions[botDirection];
				++count;
			}
			fullCommand += std::to_string(count);
			fullCommand += ',';
		}
	}
	fullCommand += (char)10;

	// TODO: Actual method to extract pattern
	std::string commands("A,C,A,B,C,A,C,B,B,C,");
	std::string A("L,12,L,12,R,4,"), B("R,6,L,12,L,12,"), C("R,10,R,6,R,4,R,4,");
	
	backup[0] = 2;
	program.Reset(backup, Program::RunMode::Silent);

	program.Run(CommandToInput(commands));
	program.Run(CommandToInput(A));
	program.Run(CommandToInput(B));
	program.Run(CommandToInput(C));
	program.Run({ (i64)'N', 10 });
	
	std::cout << program.GetOutput() << std::endl;

    return 0;
}
