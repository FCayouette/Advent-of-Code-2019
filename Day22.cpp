#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <algorithm>
#include <vector>
using i64 = long long;

// From https://www.geeksforgeeks.org/multiply-large-integers-under-large-modulo/
i64 ModuloMul(i64 a, i64 b, i64 mod)
{
    i64 result = 0;
    a %= mod;
    while (b)
    {
        if (b & 1)
            result = (result + a) % mod;
        a = (2 * a) % mod;
        b /= 2;
    }
    return result;
}

// From https://stackoverflow.com/questions/8496182/calculating-powa-b-mod-n
// Primality testing approach
// Modified to avoid multiplication overflow
i64 ModuloExp(i64 base, i64 exp, i64 mod)
{
    i64 result = 1;
    while (exp)
    {
        if (exp & 1)
            result = ModuloMul(result, base, mod);
        base = ModuloMul(base, base, mod);
        exp /= 2;
    }
    return result % mod;
}

// From https://www.rookieslab.com/posts/how-to-find-multiplicative-inverse-of-a-number-modulo-m-in-python-cpp
// Modified to avoid integer overflow on multiplications
i64 fast_power(i64 base, i64 power, i64 MOD) {
    i64 result = 1;
    while (power) {
        if (power & 1) 
            result = ModuloMul(result, base, MOD);
        base = ModuloMul(base, base, MOD);
        power /= 2;
    }
    return result;
}

i64 modulo_multiplicative_inverse(i64 A, i64 M) {
    // Assumes that M is a prime number
    // Returns multiplicative modulo inverse of A under M
    return fast_power(A, M - 2, M);
}


std::array<i64, 3> extended_euclid_gcd(i64 a, i64 b) {
    // Returns a vector `result` of size 3 where:
    // Referring to the equation ax + by = gcd(a, b)
    //     result[0] is gcd(a, b)
    //     result[1] is x
    //     result[2] is y 

    i64 s = 0, old_s = 1;
    i64 t = 1, old_t = 0;
    i64 r = b, old_r = a;

    while (r != 0) {
        i64 quotient = old_r / r;
        // We are overriding the value of r, before that we store it's current
        // value in temp variable, later we assign it to old_r
        i64 temp = r;
        r = old_r - quotient * r;
        old_r = temp;

        // We treat s and t in the same manner we treated r
        temp = s;
        s = old_s - quotient * s;
        old_s = temp;

        temp = t;
        t = old_t - quotient * t;
        old_t = temp;
    }
    std::array<i64, 3> result{ old_r, old_s, old_t };
    return result;
}

i64 modulo_multiplicative_inverse_non_prime(i64 A, i64 M) {
    // Assumes that A and M are co-prime
    // Returns multiplicative modulo inverse of A under M

    // Find gcd using Extended Euclid's Algorithm
    auto v = extended_euclid_gcd(A, M);
    //i64 gcd = v[0];
    //i64 x = v[1];
    //i64 y = v[2]; // We don't really need this though

    // In case x is negative, we handle it by adding extra M
    // Because we know that multiplicative inverse of A in range M lies
    // in the range [0, M-1]
    return v[1] < 0 ? v[1] + M : v[1];
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
        if (input[0] == 'c')
        {
            int n = std::stoi(input.substr(4));
            if (n < 0) n += deck.size();
            std::rotate(deck.begin(), deck.begin() + n, deck.end());
        }
        else if (input[5] == 'i')
            std::reverse(deck.begin(), deck.end());
        else
        {
            int n = std::stoi(input.substr(20));
            Deck oldDeck = deck;
            for (size_t i = 0, index = 0; i < DeckSize; ++i, index = (index + n) % DeckSize)
                deck[index] = oldDeck[i];
        }
        commands.push_back(input);
    }
    in.close();

    std::cout << "Part 1: " << std::find(deck.cbegin(), deck.cend(), 2019) - deck.cbegin() << std::endl;

    i64 from = 2020;
    i64 a = 1, b = 0;
    // Validation
    for (auto iter = commands.crbegin(); iter != commands.crend(); ++iter)
    {
        if ((*iter)[0] == 'c')
        {
            int n = std::stoi(iter->substr(4));
            from += (n < 0 ? n + DeckSize : n);
            from %= DeckSize;
            b += n;
        }
        else if ((*iter)[5] == 'i')
        {
            from = DeckSize - from - 1;
            a *= -1;
            b = -b - 1;
        }
        else
        {
            int n = std::stoi(iter->substr(20));
            i64 mmi = modulo_multiplicative_inverse(n, DeckSize);
            from = (from * mmi) % DeckSize;
            a = ModuloMul(a, mmi, DeckSize);
            b = ModuloMul(b, mmi, DeckSize);
        }
    }
    std::cout << deck[2020] << " " << from << " correct ? " << (deck[2020] == from ? "YES" : "NO") << std::endl;
    if (a < 0) a += DeckSize;
    if (b < 0) b += DeckSize;
    a %= DeckSize;
    b %= DeckSize;


    for (int i = 0; i < 10; ++i)
    {
        // Check if coefficients are yielding the correct results;
        i64 result = (ModuloMul(a, i, DeckSize) + b) % DeckSize;
        std::cout << deck[i] << ' ' << result << (deck[i] == result ? " YES" : " NO") << std::endl;
    }


    // Do 10 iterations and check if math is correct
    for (int i = 1; i < 10; ++i)
    {
        for (const std::string& s : commands)
        {
            if (s[0] == 'c')
            {
                int n = std::stoi(s.substr(4));
                if (n < 0) n += deck.size();
                std::rotate(deck.begin(), deck.begin() + n, deck.end());
            }
            else if (s[5] == 'i')
            {
                std::reverse(deck.begin(), deck.end());
            }
            else
            {
                int n = std::stoi(s.substr(20));
                Deck oldDeck = deck;
                for (size_t i = 0, index = 0; i < DeckSize; ++i, index = (index + n) % DeckSize)
                    deck[index] = oldDeck[i];
            }
        }
    }
    for (int j = 0; j < 10; ++j)
    {
        i64 t1 = ModuloExp(a, 10, DeckSize);
        i64 t2 = ModuloMul(t1, j, DeckSize);
        i64 t3 = (t1 + DeckSize - 1) % DeckSize;
        i64 t4 = ModuloMul(b, t3, DeckSize);
        i64 t5 = ModuloExp(a - 1, DeckSize - 2, DeckSize);

        i64 result = (t2 + ModuloMul(t4, t5, DeckSize)) % DeckSize;
        std::cout << deck[j] << ' ' << result << (deck[j] == result ? " YES" : " NO") << std::endl;
    }

    // Part 2 
    i64 dSize = 119315717514047;
    i64 shuffles = 101741582076661;

    // Reset computations
    a = 1;
    b = 0;
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
            i64 mmi = modulo_multiplicative_inverse(n, dSize);
            a = ModuloMul(a, mmi, dSize);
            b = ModuloMul(b, mmi, dSize);
        }
    }
    if (a < 0) a += dSize;
    if (b < 0) b += dSize;
    a %= dSize;
    b %= dSize;
    // Apply the reverse effect {shuffles} times through modular exponentiation
    i64 t1 = ModuloExp(a, shuffles, dSize);
    i64 t2 = ModuloMul(t1, 2020, dSize);
    i64 t3 = (t1 + dSize - 1) % dSize;
    i64 t4 = ModuloMul(b, t3, dSize);
    i64 t5 = ModuloExp(a - 1, dSize - 2, dSize);

    i64 result = (t2 + ModuloMul(t4, t5, dSize)) % dSize;
    std::cout << "Part 2: " << result << std::endl;

    return 0;
}
