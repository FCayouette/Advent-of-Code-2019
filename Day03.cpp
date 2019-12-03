#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

struct Segment
{
	int sX, sY, eX, eY, lengthAtStart;
	char dir;

	void InitSegment(const std::string& s, int& x, int& y, int& length)
	{
		dir = s[0];
		int dx = 0, dy = 0;
		switch (dir)
		{
		case 'U': dy = 1; break;
		case 'L': dx = -1; break;
		case 'R': dx = 1; break;
		default: dy = -1; break;
		}

		x += dx;
		y += dy;
		sX = x;
		sY = y;
		lengthAtStart = ++length;
		int wireLength = atoi(s.substr(1).c_str()) - 1; // We already shifted by one
		x += dx * wireLength;
		y += dy * wireLength;
		eX = x;
		eY = y;
		length += wireLength;
	}

	bool Intersect(const Segment& s, int& manhattan, int& wireLengthAtCross)
	{
		switch (dir)
		{
		case 'U':
		case 'D':
			if (s.dir == 'U' || s.dir == 'D' ||
				std::min(sY, eY) > s.sY || std::max(sY, eY) < s.sY ||
				std::min(s.sX, s.eX) > sX || std::max(s.sX, s.eX) < sX)
				return false;
			{
				int colX = sX, colY = s.sY;
				manhattan = std::abs(colX) + std::abs(colY);
				wireLengthAtCross = lengthAtStart + s.lengthAtStart + std::abs(colX - s.sX) + std::abs(colY - sY);
				return true;
			}
			break;
		default:
			if (s.dir == 'L' || s.dir == 'R' ||
				std::min(sX, eX) > s.sX || std::max(sX, eX) < s.sX ||
				std::min(s.sY, s.eY) > sY || std::max(s.sY, s.eY) < sY)
				return false;
			{
				int colX = s.sX, colY = sY;
				manhattan = std::abs(colX) + std::abs(colY);
				wireLengthAtCross = lengthAtStart + s.lengthAtStart + std::abs(colX - sX) + std::abs(colY - s.sY);
				return true;
			}
		}
	}
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

	// Process first wire as a list of segments
	std::string wireDescription;
	in >> wireDescription;

	std::vector<Segment> firstWire;
	size_t offset = 0, result = 0;
	int x = 0, y = 0, length = 0;
	do
	{
		result = wireDescription.find(',', offset);
		Segment segment;
		if (result != std::string::npos)
			segment.InitSegment(wireDescription.substr(offset, result - offset), x, y, length);
		else
			segment.InitSegment(wireDescription.substr(offset), x, y, length);
		firstWire.push_back(segment);
		offset = result + 1;
	} while (result != std::string::npos);

	// Process second wire and check for collisions
	in >> wireDescription;
	
	int manhattan = length, combinePath = length * 2;
	offset = result = 0;
	x = y = length = 0;
	do
	{
		result = wireDescription.find(',', offset);
		Segment segment;
		if (result != std::string::npos)
			segment.InitSegment(wireDescription.substr(offset, result - offset), x, y, length);
		else
			segment.InitSegment(wireDescription.substr(offset), x, y, length);

		// Check current segment with all the first wire segments
		for (const Segment& other : firstWire)
		{
			int m, c;
			if (segment.Intersect(other, m, c))
			{
				manhattan = std::min(m, manhattan);
				combinePath = std::min(c, combinePath);
			}
		}
		offset = result + 1;
	} while (result != std::string::npos);

	in.close();

	std::cout << "Part 1: " << manhattan << std::endl << "Part 2: " << combinePath << std::endl;
	return 0;
}