#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
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

    bool Uses(const std::string& reagentName) const
    {
        return std::find_if(inputs.cbegin(), inputs.cend(), [reagentName](const Reagent& r) { return r.name == reagentName; }) != inputs.cend();
    }
    bool operator<(const Reaction& r) const { return output.name < r.output.name; }

    std::vector<Reagent> inputs;
    Reagent output;
 };

using Stock = std::map<std::string, i64>;
using ReactionMap = std::set<Reaction>;

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

    ReactionMap reactions;
    std::string input;

    while (std::getline(in, input))
        reactions.emplace(input);
    in.close();

    Stock requiredStocks;
    std::vector<Reaction> reactionChain;
    
    // Perform first part
    requiredStocks["FUEL"] = 1;    
    Reaction desiredReaction; // Used to search reactions
    while (!reactions.empty())
        for (auto stockIter = requiredStocks.begin(); stockIter != requiredStocks.end(); ++stockIter)
        {
            const std::string& name = stockIter->first;
            auto reactionIter = reactions.cbegin();
            for (; reactionIter != reactions.cend(); ++reactionIter)
                if (reactionIter->Uses(name))
                    break;
            if (reactionIter == reactions.cend()) // Nothing else uses this reagent, therefore we know we have the correct required amount
            {
                desiredReaction.output.name = name;
                ReactionMap::iterator currentReaction = reactions.find(desiredReaction);
                i64 times = (stockIter->second + currentReaction->output.amount - 1) / currentReaction->output.amount;
                // Add each inputs to requiredStocks
                for (const Reagent& reagent : currentReaction->inputs)
                    requiredStocks[reagent.name] += times * reagent.amount;

                reactionChain.push_back(*currentReaction); // Build the reaction chain for part 2
                reactions.erase(currentReaction);
                requiredStocks.erase(name);
                break;
            }
        }
    std::cout << "ORE required: " << requiredStocks["ORE"] << std::endl;

    // Part 2: Perform binary search on the amount of fuel
    i64 low = trillion / requiredStocks["ORE"];
    i64 high = low * 10;
    i64 current = (low + high) / 2;
    do
    {
        for (auto& stock : requiredStocks)
            stock.second = 0;
        requiredStocks["FUEL"] = current;
        for (const Reaction& reaction : reactionChain)
        {
            i64 times = (requiredStocks[reaction.output.name] + reaction.output.amount - 1) / reaction.output.amount;
            for (const Reagent& reagent : reaction.inputs)
                requiredStocks[reagent.name] += times * reagent.amount;
        }

        if (requiredStocks["ORE"] <= trillion)
            low = current + 1;
        else
            high = current - 1;
        current = (low + high) / 2;
        
    } while (high >= low);
    
    std::cout << current << " FUEL produced" << std::endl;

    return 0;
}