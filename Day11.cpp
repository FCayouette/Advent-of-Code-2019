#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "TemplatedUtilities.h"

using i64 = long long;
using IntCode = std::map<i64, i64>;

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
	void Reset(const IntCode& newProgram, bool pausable) { pc = output = relative = 0; program = newProgram; breakOnOutput = pausable; }
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
				if (breakOnOutput) pause = true;
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
	bool breakOnOutput = false;
};

using ShipSurface = std::map<Point, i64>;
void PrintPattern(ShipSurface& surface)
{
	Boundaries<int> bounds;
	for (const auto& s : surface) 
		bounds += s.first;
	for (int i = bounds.minX; i <= bounds.maxX; ++i)
	{
		for (int j = bounds.minY; j <= bounds.maxY; ++j)
			std::cout << (surface[Point(i, j)] == 0 ? ' ' : '#');
		std::cout << std::endl;
	}
}

void MovePaintBot(Program& program, ShipSurface& surface, Point currentPosition)
{
	int direction = 2;
	bool completed = false;
	do
	{
		if (program.Run({ surface[currentPosition] }))
			break;
		surface[currentPosition] = program.GetOutput();
		completed = program.Run({});
		direction = (direction + (program.GetOutput() == 1 ? 3 : 1)) % 4;
		currentPosition += directions[direction];
		if (surface.find(currentPosition) == surface.cend())
			surface[currentPosition] = 0;
	} while (!completed);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage Day11.exe [filename]" << std::endl;
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
	program.Reset(backupProgram, true);

	ShipSurface surface;
	Point position(0, 0);
	surface[position] = 0;
	MovePaintBot(program, surface, position);

	std::cout << "Part 1: " << surface.size()-1 << std::endl; // -1 because we never paint the last panel
	// Printing pattern just for fun
	PrintPattern(surface);
	
	// Part 2
	surface.clear();
	program.Reset(backupProgram, true);
	
	surface[position] = 1;
	MovePaintBot(program, surface, position);
		
	std::cout << "Part 2: " << std::endl;
	PrintPattern(surface);
    return 0;
}