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
				// Special code for networking
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
					std::cout << output << std::endl;
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

struct Packet
{
	Packet(i64 x=-1, i64 y=-1) : X(x), Y(y) {}
	i64 X, Y;
};

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: Day23.exe [filename]" << std::endl;
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

	std::vector<Program> network;
	std::array<std::deque<Packet>,50> inFlightPackets, generatedPackets;
	network.reserve(50);
	for (int i = 0; i < 50; ++i)
	{
		network.emplace_back(backup, Program::RunMode::BreakOnOutput);
		network.back().Run({ i }); // Only do init at first
	}
	
	bool foundPart1 = false;
	int iterations = 0;
	Packet NATpacket, lastSentNATpacket(0, 0x8000000000000000);
	while (true)
	{
		size_t idleCount = 0;
		for (int i = 0; i < 50; ++i)
		{
			Program& currentNode = network[i];

			std::vector<i64> inputs;
			if (currentNode.WaitingForInput())
			{
				if (!inFlightPackets[i].empty())
				{
					inputs.push_back(inFlightPackets[i].front().X);
					inputs.push_back(inFlightPackets[i].front().Y);
					inFlightPackets[i].pop_front();
				}
				else
					inputs.push_back(-1);
			}

			;
			if (currentNode.Run(inputs) == Program::ReturnState::WaitingOnOutput)
			{
				i64 address = currentNode.GetOutput();
				currentNode.Run({});
				i64 x = currentNode.GetOutput();
				currentNode.Run({});
				i64 y = currentNode.GetOutput();
				if (address == 255)
				{
					NATpacket.X = x;
					NATpacket.Y = y;
					if (!foundPart1)
					{
						foundPart1 = true;
						std::cout << "Part 1: " << y << std::endl;
					}
				}
				else
					inFlightPackets[static_cast<size_t>(address)].emplace_back(x, y);
			}
			else if (inputs.size() == 1)
				++idleCount;
		}
		if (idleCount == 50)
		{
			if (NATpacket.Y == lastSentNATpacket.Y)
			{
				std::cout << "Part 2: " << NATpacket.Y << std::endl;
				break;
			}
			inFlightPackets[0].push_back(NATpacket);
			lastSentNATpacket = NATpacket;
		}
	}

	return 0;
}
