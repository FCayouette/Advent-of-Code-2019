#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>

struct Wire
{
	Wire(const std::string& s)
	{
		dir = s[0];
		length = atoi(s.substr(1).c_str());
	}
	char dir;
	int length;
};

struct Point
{
	int x, y;
	Point(int X=0, int Y=0) : x(X), y(Y) {}
	bool operator<(const Point& p) const { return x < p.x || (x == p.x && y < p.y); }
};

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: Day3.exe [filename]" << std::endl;
		return -1;
	}

	std::fstream in(argv[1], std::ios::in);
	if (!in)
		return -1;

	// Read data
	std::string input;
	in >> input;
	std::vector<Wire> wire1, wire2;
	size_t offset = 0, result = 0;

	do 
	{
		result = input.find_first_of(',', offset);
		if (result != std::string::npos)
			wire1.emplace_back(input.substr(offset, result - offset));
		else
			wire1.emplace_back(input.substr(offset));
		offset = result + 1;

	} while (result != std::string::npos);

	in >> input;
	offset = result = 0;
	do
	{
		result = input.find_first_of(',', offset);
		if (result != std::string::npos)
			wire2.emplace_back(input.substr(offset, result - offset));
		else
			wire2.emplace_back(input.substr(offset));
		offset = result + 1;

	} while (result != std::string::npos);

	in.close();

	std::set<Point> wirePath;
	std::map<Point, int> wirePathLength;
	int x = 0, y = 0, length = 0;
	for (const auto& w : wire1)
	{
		int dx = 0, dy = 0;
		switch (w.dir)
		{
		case 'R': dx = 1; break;
		case 'D': dy = 1; break;
		case 'L': dx = -1; break;
		case 'U': dy = -1; break;
		}
		for (int i = 0; i < w.length; ++i)
		{
			x += dx;
			y += dy;
			++length;
			
			wirePath.emplace(x, y);
			Point p(x, y);
			if (wirePathLength.find(p) == wirePathLength.cend())
				wirePathLength[p] = length;
		}
	}

	x = y = length = 0;
	int lowest = wirePath.size(), lowestCombine = wirePath.size() * 2;
	for (const Wire& w : wire2)
	{
		int dx = 0, dy = 0;
		switch (w.dir)
		{
		case 'R': dx = 1; break;
		case 'D': dy = 1; break;
		case 'L': dx = -1; break;
		case 'U': dy = -1; break;
		}
		for (int i = 0; i < w.length; ++i)
		{
			x += dx;
			y += dy;
			++length;
			Point p(x, y);
			if (wirePath.find(p) != wirePath.cend())
			{
				int result = std::abs(x) + std::abs(y);
				if (result < lowest)
					lowest = result;
				int combine = length + wirePathLength[p];
				if (combine < lowestCombine)
					lowestCombine = combine;
			}
		}
	}

	std::cout << "Part 1: " << lowest << std::endl << "Part 2: " << lowestCombine << std::endl;
	return 0;
}
