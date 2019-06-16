#include "utils.hpp"

#include <cctype>

using namespace xsteg;

void str_toupper(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char ch)
    {
        return std::toupper(ch);
    });
}

pixel_availability parse_px_availability_bits(const std::string& bits_str)
{
    pixel_availability result;
    int ibits = std::stoi(bits_str);
    result.r = ibits / 1000;
    result.g = (ibits % 1000) / 100;
    result.b = (ibits % 100) / 10;
    result.a = (ibits % 10);
    return result;
}

std::vector<uint8_t> str_to_datavec(const std::string& str)
{
    std::vector<uint8_t> result;
    result.resize(str.size(), 0x00u);
    std::transform(str.begin(), str.end(), result.begin(), [](char ch)
    {
        return static_cast<uint8_t>(ch);
    });
    return result;
}