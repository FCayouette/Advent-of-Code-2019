#include <iostream>
#include <fstream>
#include <string>
#include "TemplatedUtilities.h"
#include <vector>
#include <map>
#include <set>
#include <array>

using Area = std::vector<std::string>;
using ItemMap = std::map<char, Point>;

struct SearchState
{
	SearchState() : distanceTravelled(0), path("@") {} // Always starts at @
	std::string available;
	std::string path;
	int distanceTravelled;
	bool operator<(const SearchState& s) const { 
		return distanceTravelled < s.distanceTravelled || (distanceTravelled == s.distanceTravelled && 
			(path.size() > s.path.size() || (path.size() == s.path.size() && path < s.path)));
	}
	
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
		return -1;

	std::string input;
	std::vector<std::string> area;
	std::map<char, Point> keys;
	std::map<char, Point> doors;
	Point entrance;
	while (in >> input)
	{
		for (size_t i = 0; i < input.size(); ++i)
		{
			char c = input[i];
			if (c == '@')
			{
				entrance = Point(i, area.size());
				doors[c] = entrance; // simplifies things
			}
			else if (c >= 'a' && c <= 'z')
				keys[c] = Point(i, area.size());
			else if (c >= 'A' && c <= 'Z')
				doors[c] = Point(i, area.size());
		}
		area.push_back(input);
	}
	in.close();

	std::map<std::pair<char, char>, int> costsFromKeyToKey; // also from entrance to keys
	std::map<char, std::string> makesAvailable;
	std::string doorsToEvaluate = "@";

	Bounds bounds(Point(0, 0));
	bounds += Point(area[0].size() - 1, area.size() - 1);

	// Find initial accessible keys
	std::set<Point> testedPoints;
	testedPoints.insert(entrance);
	int iteration = 1;
	while (!doorsToEvaluate.empty())
	{
		char currentDoor = doorsToEvaluate.front();
		doorsToEvaluate = doorsToEvaluate.substr(1);
		std::set<Point> currentIter, previousIter;
		previousIter.insert(doors[currentDoor]);
		do
		{
			for (const Point& p : previousIter)
				for (int i = 0; i < 4; ++i)
				{
					Point np = p + directions[i];
					if (bounds.Inside(np) && testedPoints.find(np) == testedPoints.cend())
					{
						testedPoints.insert(np);
						char c = area[np.y][np.x];
						if (c != '#')
						{
							if (c == '.') // Open space
								currentIter.insert(np);
							else if (c >= 'a' && c <= 'z') // a key
							{
								makesAvailable[currentDoor] += c;
								currentIter.insert(np);
							}
							else if (c >= 'A' && c <= 'Z')
								doorsToEvaluate += c; // Stop fill at door
						}
					}
				}
			std::swap(currentIter, previousIter);
			currentIter.clear();
			++iteration;
		} while (!previousIter.empty());
	}

	// Now for each key pair compute the distance, adding @ as a key for complete info
	keys['@'] = doors['@'];

	for (const auto& keyData : keys)
	{
		testedPoints.clear();
		testedPoints.insert(keyData.second);
		std::set<Point> currentIter, previousIter;
		previousIter.insert(keyData.second);
		iteration = 1;
		do
		{
			for (const Point& p : previousIter)
				for (int i = 0; i < 4; ++i)
				{
					Point np = p + directions[i];
					if (bounds.Inside(np) && testedPoints.find(np) == testedPoints.cend())
					{
						testedPoints.insert(np);
						char c = area[np.y][np.x];
						if (c != '#')
						{
							currentIter.insert(np);
							if (c >= 'a' && c <= 'z') // a key
								costsFromKeyToKey[std::make_pair(keyData.first, c)] = iteration;
							
						}
					}
				}
			std::swap(currentIter, previousIter);
			currentIter.clear();
			++iteration;
		} while (!previousIter.empty());
	}
	keys.erase(keys.find('@')); // Do not keep it in key set

	std::map<std::string, int> bestPathToReachState;
	SearchState initialState;
	initialState.available = makesAvailable['@'];
	std::set<SearchState> breadthFirstSearch;
	breadthFirstSearch.insert(initialState);

	do
	{
		SearchState current = *breadthFirstSearch.begin();
		breadthFirstSearch.erase(breadthFirstSearch.begin());

		char currentAt = current.path.back();
		for (char c : current.available)
		{
			SearchState newState = current;
			newState.distanceTravelled += costsFromKeyToKey[std::make_pair(currentAt, c)];

			// Compute end state string
			std::string endState = newState.path.substr(1); // avoid '@';
			std::sort(endState.begin(), endState.end());
			endState += c;
			if (auto iter = bestPathToReachState.find(endState);
				iter != bestPathToReachState.cend() && iter->second <= newState.distanceTravelled) // Eliminate duplicates
				continue;
			bestPathToReachState[endState] = newState.distanceTravelled;

			newState.path += c;
			newState.available.erase(newState.available.find(c), 1);
			newState.available += makesAvailable[c - 0x20];
			breadthFirstSearch.insert(newState);
		}
	} while (breadthFirstSearch.begin()->available.size() != 0);

	std::cout << "Part 1: " << breadthFirstSearch.cbegin()->distanceTravelled << " with path " << breadthFirstSearch.cbegin()->path << std::endl;
	bestPathToReachState.clear();
	breadthFirstSearch.clear();

	// Set up part 2, determine which keys are in which quadrant, same with doors
	std::array<std::string, 4> keysInQuadrant, doorsInQuadrant;
	for (auto k : keys)
	{
		int q = 0;
		if (k.second.y > entrance.y)
			q += 2;
		if (k.second.x > entrance.x)
			++q;
		keysInQuadrant[q] += k.first;
	}
	for (auto d : doors)
	{
		int q = 0;
		if (d.second.y > entrance.y)
			q += 2;
		if (d.second.x > entrance.x)
			++q;
		doorsInQuadrant[q] += d.first;
	}

	int total = 0;
	int qIndex = 0;
	for (const std::string quadrant : keysInQuadrant)
	{
		// Solve each quadrant independently
		// Check if there are doors and key pairs in the quadrant
		// All other doors will be opened by another bot at some time
		std::string available = quadrant;
		for (char a : quadrant)
			if (doorsInQuadrant[qIndex].find(a - 0x20) != std::string::npos)
			{
				std::string tmp = makesAvailable[a - 0x20];
				for (char c : tmp)
					if (size_t index = available.find(c);
						index != std::string::npos)
						available.erase(index,1);
			}
		
		std::map<std::string, int> quadrantBestPaths;
		SearchState init;
		init.available = available;
		std::set<SearchState> bfs;
		bfs.insert(init);

		do
		{
			SearchState current = *bfs.begin();
			bfs.erase(bfs.begin());
	
			char currentAt = current.path.back();
			for (char c : current.available)
			{
				SearchState newState = current;
				newState.distanceTravelled += costsFromKeyToKey[std::make_pair(currentAt, c)];
				
				// Compute end state string
				std::string endState = newState.path.substr(1); // avoid '@';
				std::sort(endState.begin(), endState.end());
				endState += c;
				if (auto iter = quadrantBestPaths.find(endState);
					iter != quadrantBestPaths.cend() && iter->second <= newState.distanceTravelled) // Eliminate duplicates
					continue;
				
				quadrantBestPaths[endState] = newState.distanceTravelled;
	
				newState.path += c;
				newState.available.erase(newState.available.find(c), 1);
				for (char c : makesAvailable[c - 0x20])
					if (quadrant.find(c) != std::string::npos)
						newState.available += c;
				bfs.insert(newState);
			}
		} while (bfs.begin()->available.size() != 0);
		total += bfs.begin()->distanceTravelled - 2; // Since we started 2 points farther at center

		++qIndex;
	}

	std::cout << "Part 2: " << total << std::endl;

	return 0;
}

