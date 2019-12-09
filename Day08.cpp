#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>

constexpr size_t width = 25;
constexpr size_t height = 6;
constexpr size_t stride = width * height;

struct Layer
{
    std::array<int, 3> histogram = { 0, 0, 0 };
    Layer(const std::string& str, size_t startIndex) 
    {
        for (size_t i = 0; i < stride; ++i )
            ++histogram[str[startIndex + i] - '0'];
    }
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: Day08.exe [filename]" << std::endl;
        return -1;
    }

    std::ifstream in(argv[1], std::ios::in);
    if (!in)
        return -1;

    std::string image;
    in >> image;

    size_t index = 0;
    std::vector<Layer> layers;
    do 
    {
        layers.emplace_back(image, index);
        index += stride;
    } while (index < image.size());

    
    int lowest0 = stride, bestIndex = -1;
    for (std::vector<Layer>::const_iterator iter = layers.cbegin(); iter != layers.cend(); ++iter)
        if (iter->histogram[0] < lowest0)
        {
            lowest0 = iter->histogram[0];
            bestIndex = iter - layers.cbegin();
        }

    std::cout << "Part 1: " << layers[bestIndex].histogram[1] * layers[bestIndex].histogram[2] << std::endl << std::endl << "Part 2" << std::endl;

    size_t pixelIndex = 0;
    for (size_t i = 0; i < height; ++i)
    {
        for (size_t j = 0; j < width; ++j, ++pixelIndex)
        {
            size_t p = pixelIndex;
            while (true)
            {
                if (image[p] != '2')
                    break;
                p += stride;
            }
            std::cout << (image[p] == '0' ? ' ' : '#');
        }
        std::cout << std::endl;
    }
    return 0;
}