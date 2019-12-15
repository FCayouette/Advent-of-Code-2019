#include <iostream>
#include <fstream>
#include <string>
#include "TemplatedUtilities.h"
#include <map>
#include <deque>

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
		Continuous,
		BreakOnOutput,
	} mode;

	Program(const IntCode& newProgram, RunMode m = RunMode::Continuous) { Reset(newProgram, m); }
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
				if (mode == RunMode::BreakOnOutput) pause = true;
				else std::cout << output << std::endl;
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

struct PathInfo
{
	PathInfo(Program& prog, P64 p = P64(0, 0), i64 d = 0) : program(prog), position(p), distance(d) {}
	Program program;
	P64 position;
	i64 distance;
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: Day15.exe [filename]" << std::endl;
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

	Program program(backupProgram, Program::RunMode::BreakOnOutput);

	std::set<P64> testedPoints;

	// Initialize search
	PathInfo origin(program);
	testedPoints.insert(P64(0, 0));
	std::deque<PathInfo> search = { origin };
	std::set<P64> openLocations;
	openLocations.insert(P64(0,0));

	constexpr P64 delta[] = {P64(0,0), P64(0,-1), P64(0,1), P64(-1,0), P64(1,0) };

	i64 distance = -1;
	P64 generator;
	do
	{
		PathInfo& info = search.front();
		for (i64 i = 1; i < 5; ++i)
		{
			P64 newPos = info.position + delta[i];
			if (testedPoints.find(newPos) == testedPoints.cend())
			{
				testedPoints.insert(newPos);
				PathInfo pi(info.program, newPos, info.distance + 1);
				pi.program.Run({ i });
				if (pi.program.GetOutput() == 2)
				{
					std::cout << "Distance to generator: " << pi.distance << std::endl;
					generator = pi.position;
				}
				if (pi.program.GetOutput() == 1)
				{
					openLocations.insert(pi.position);
					search.push_back(pi);
				}
			}
		}
		search.pop_front();
	} while (!search.empty());

	// Part 2: Fill the area from the 02 generator
	std::set<P64> justFilledArea = { generator };
	openLocations.erase(generator);
	int iterations = 0;
	while (!openLocations.empty())
	{
		++iterations;
		std::set<P64> nextIter;

		for (const P64& p : justFilledArea)
			for (int i = 1; i < 5; ++i)
			{
				P64 newPos = p + delta[i];
				if (auto iter = openLocations.find(newPos);
					iter != openLocations.cend())
				{
					nextIter.insert(newPos);
					openLocations.erase(iter);
				}
			}

		std::swap(nextIter, justFilledArea);
	}
	std::cout << "Ship filled in " << iterations << " minutes" << std::endl;
	return 0;
}
