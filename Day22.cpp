#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <algorithm>
#include <vector>
#include "TemplatedUtilities.h"

using i64 = long long;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage Day22.exe [filename]" << std::endl;
        return -1;
    }

    std::ifstream in(argv[1], std::ios::in);
    if (!in)
        return -1;

    // Initialize deck
    constexpr int DeckSize = 10007;
    using Deck = std::array<int, DeckSize>;
    Deck deck;
    for (int i = 0; i < DeckSize; ++i)
        deck[i] = i;

    std::string input;
    std::vector<std::string> commands;
    while (std::getline(in, input))
    {
        if (input[0] == 'c') // Cut N instruction
        {
            int n = std::stoi(input.substr(4));
            if (n < 0) n += deck.size();
            std::rotate(deck.begin(), deck.begin() + n, deck.end());
        }
        else if (input[5] == 'i') // Deal into new stack instruction
            std::reverse(deck.begin(), deck.end());
        else
        {   // Deal with increment N instruction
            int n = std::stoi(input.substr(20));
            Deck oldDeck = deck;
            for (size_t i = 0, index = 0; i < DeckSize; ++i, index = (index + n) % DeckSize)
                deck[index] = oldDeck[i];
        }
        commands.push_back(input);
    }
    in.close();

    std::cout << "Part 1: " << std::find(deck.cbegin(), deck.cend(), 2019) - deck.cbegin() << std::endl;

    // Part 2 
    constexpr i64 largeDeckSize = 119315717514047; // Assumed to be prime
    constexpr i64 shuffles = 101741582076661;

    i64 a = 1, b = 0;
    // Compute reverse effect of a single shuffle as a  A*x + B effect
    for (auto iter = commands.crbegin(); iter != commands.crend(); ++iter)
    {
        if ((*iter)[0] == 'c')
        {
            i64 n = std::stoi(iter->substr(4));
            b += n;
        }
        else if ((*iter)[5] == 'i')
        {
            a *= -1;
            b = -b - 1;
        }
        else
        {
            i64 n = std::stoi(iter->substr(20));
            i64 mmi = ModuloInvMul(n, largeDeckSize);
            a = ModuloMul(a, mmi, largeDeckSize);
            b = ModuloMul(b, mmi, largeDeckSize);
        }
    }
    if (a < 0) a += largeDeckSize;
    if (b < 0) b += largeDeckSize;
    a %= largeDeckSize;
    b %= largeDeckSize;
    // Apply the reverse effect {shuffles} times through modular exponentiation
    // One iteration as effect x*A + b
    // 2 iterations are of effect x*A^2 +A*b + b;
    // N iterations give the form x*A^N + b*A^N-1 + b*A^N-2 + ... + b*A + b
    // which can be reduced to x*A^N + b*SumOf(A^i) with i from 0 to N-1
    // Which is equal to x*A^N + b * (1-A^N) / (1 - A)
     
    i64 aPowShuffles = ModuloExp(a, shuffles, largeDeckSize);
    i64 frontTerm = ModuloMul<i64>(aPowShuffles, 2020, largeDeckSize); // 2020 is the target index
    i64 aPowShufflesMinusOne = (aPowShuffles + largeDeckSize - 1) % largeDeckSize; 
    i64 secondTerm = ModuloMul(b, aPowShufflesMinusOne, largeDeckSize); // 1-x and x-1 are congruent in modulo calculus
    i64 secondTermDividend = ModuloInvMul(a - 1, largeDeckSize);

    i64 result = (frontTerm + ModuloMul(secondTerm, secondTermDividend, largeDeckSize)) % largeDeckSize; // Combine second term and add first
    std::cout << "Part 2: " << result << std::endl;

    return 0;
}
