#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using i64 = long long;
constexpr i64 trillion = 1000000000000;

struct Reagent
{
    Reagent() : amount(0) {}
    Reagent(const std::string& s)
    {
        size_t index = s.find(' ');
        amount = std::stoll(s.substr(0, index));
        name = s.substr(index + 1);
    }
    Reagent& operator=(const Reagent&) = default;

    std::string name;
    i64 amount;
};

struct Reaction
{
    Reaction() = default;
    Reaction(const std::string& s)
    {
        size_t index = 0, start = 0;
        while ((index = s.find(',', start)) != std::string::npos)
        {
            inputs.emplace_back(s.substr(start, index - start));
            start = index + 2; // skip space
        }
        index = s.find('=', start);
        inputs.emplace_back(s.substr(start, index - start - 1));

        output = Reagent(s.substr(index + 3));
    }

    std::vector<Reagent> inputs;
    Reagent output;
    bool Uses(const std::string& s) const
    {
        return std::find_if(inputs.cbegin(), inputs.cend(), [s](const Reagent& r) { return r.name == s; }) != inputs.cend();
    }
    bool operator<(const Reaction& r) const { return output.name < r.output.name; }
 };

using Stock = std::map<std::string, i64>;
using ReactionMap = std::map<std::string, Reaction>; // First the name of the output

void PerformReaction(ReactionMap& reactions, Stock& requiredStocks)
{
    while (!reactions.empty())
    {
        for (auto iter = requiredStocks.begin(); iter != requiredStocks.end(); ++iter)
        {
            const std::string& s = iter->first;
            auto r = reactions.cbegin();
            for (; r != reactions.cend(); ++r)
                if (r->second.Uses(s))
                    break;
            if (r == reactions.cend()) // Nobody else uses this reagent, therefore we know we have the correct required amount
            {
                // Determine the amount of time the reaction has to be executed
                const Reaction& currentReaction = reactions[s];
                i64 times = (iter->second + currentReaction.output.amount - 1) / currentReaction.output.amount;
                // Add each inputs to requiredStocks
                for (const Reagent& reagent : currentReaction.inputs)
                    requiredStocks[reagent.name] += times * reagent.amount;

                reactions.erase(s);
                requiredStocks.erase(s);
                break;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: Day14.exe [filename]" << std::endl;
        return -1;
    }

    std::ifstream in(argv[1], std::ios::in);
    if (!in)
        return -1;

    ReactionMap reactions, backup;
    std::string input;

    while (std::getline(in, input))
    {
        // Parse reaction
        Reaction r(input);
        reactions[r.output.name] = input;
    }
    in.close();

    backup = reactions;
    std::map<std::string, i64> requiredStocks;
    
    // Perform first part
    requiredStocks["FUEL"] = 1;    
    PerformReaction(reactions, requiredStocks);
    std::cout << "ORE required: " << requiredStocks["ORE"] << std::endl;

    // Part 2: Perform binary search on the amount of fuel
    i64 low = trillion / (i64)requiredStocks["ORE"];
    i64 high = low * 10;
    i64 current = (low + high) / 2;
    do
    {
        reactions = backup;
        requiredStocks.clear();
        requiredStocks["FUEL"] = current;
        PerformReaction(reactions, requiredStocks);

        i64 oreLeft = trillion - requiredStocks["ORE"];
        if (oreLeft > 0)
            low = current + 1;
        else
            high = current - 1;
        current = (low + high) / 2;
        
    } while (high >= low);
    
    std::cout << current << " FUEL produced" << std::endl;

    return 0;
}