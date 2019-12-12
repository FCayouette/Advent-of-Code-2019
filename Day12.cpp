#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <numeric>

using i64 = long long;

struct Moon
{
	std::array<int, 3> pos, oPos, vel;

	Moon(const std::string& s) : vel({ 0,0,0 })
	{
		sscanf_s(s.c_str(), "<x=%d, y=%d, z=%d>", &pos[0], &pos[1], &pos[2]);
		oPos = pos;
	}

	void ApplyMutualGravity(Moon& o)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (pos[i] < o.pos[i])
			{
				++vel[i];
				--o.vel[i];
			}
			else if (pos[i] > o.pos[i])
			{
				--vel[i];
				++o.vel[i];
			}
		}
	}

	void ApplyVelocity() // returns true if we think we are in a cycle
	{
		for (int i = 0; i < 3; ++i)
			pos[i] += vel[i];
	}

	int PotentialEnergy() const
	{
		int p = 0, v = 0;
		for (int i = 0; i < 3; ++i)
		{
			p += std::abs(pos[i]);
			v += std::abs(vel[i]);
		}
		return p * v;
	}
};

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: Day12.exe []" << std::endl;
		return -1;
	}

	std::ifstream in(argv[1], std::ios::in);
	if (!in)
		return -1;

	std::vector<Moon> moons;
	std::string input, completeInput;
	while (std::getline(in, input))
		moons.emplace_back(input);
	in.close();
	
	std::array<i64, 3> axis = { 0, 0, 0 };
	for (i64 iter = 1; iter <= 1000 || std::find(axis.cbegin(), axis.cend(), 0) != axis.cend(); ++iter)
	{
		for (size_t i = 0; i < moons.size(); ++i)
			for (size_t j = i + 1; j < moons.size(); ++j)
				moons[i].ApplyMutualGravity(moons[j]);
		for (Moon& m : moons)
			m.ApplyVelocity();
		for (int i = 0; i < 3; ++i)
			if (axis[i]==0)
			{
				int count = 0;
				for (Moon& m : moons)
				{
					if (m.pos[i] == m.oPos[i])
						++count;
					else break;
				}
				if (count == moons.size())
					axis[i] = iter+1;
			}

		if (iter == 1000)
		{
			int potential = 0;
			for (const Moon& m : moons)
				potential += m.PotentialEnergy();

			std::cout << "Potential Energy " << potential << std::endl;
		}
	}

	std::cout << "Part 2: " << std::lcm(axis[0], std::lcm(axis[1], axis[2])) << std::endl;
	return 0;
}