#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <algorithm>
#include <vector>
using i64 = long long;

// From https://www.geeksforgeeks.org/multiply-large-integers-under-large-modulo/
constexpr i64 ModuloMul(i64 a, i64 b, i64 mod)
{
    i64 result = 0;
    a %= mod;
    while (b)
    {
        if (b & 1)
            result = (result + a) % mod;
        a = (2 * a) % mod; // assumes this doesn't overflow
        b /= 2;
    }
    return result;
}

// From https://stackoverflow.com/questions/8496182/calculating-powa-b-mod-n
// Primality testing approach
// Modified to avoid multiplication overflow
constexpr i64 ModuloExp(i64 base, i64 exp, i64 mod)
{
    i64 result = 1;
    while (exp)
    {
        if (exp & 1)
            result = ModuloMul(result, base, mod);
        base = ModuloMul(base, base, mod);
        exp /= 2;
    }
    return result;
}

// From https://www.rookieslab.com/posts/how-to-find-multiplicative-inverse-of-a-number-modulo-m-in-python-cpp
constexpr i64 ModuloInvMul(i64 A, i64 M) {
    // Assumes that M is a prime number
    // Returns multiplicative modulo inverse of A under M
    return ModuloExp(A, M - 2, M);
}

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
    // Compute reverse effect of a single shuffle as a  A*{index} + B effect
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
    i64 t1 = ModuloExp(a, shuffles, largeDeckSize);
    i64 t2 = ModuloMul(t1, 2020, largeDeckSize); // 2020 is the target index
    i64 t3 = (t1 + largeDeckSize - 1) % largeDeckSize;
    i64 t4 = ModuloMul(b, t3, largeDeckSize);
    i64 t5 = ModuloInvMul(a - 1, largeDeckSize);

    i64 result = (t2 + ModuloMul(t4, t5, largeDeckSize)) % largeDeckSize;
    std::cout << "Part 2: " << result << std::endl;

    return 0;
}
