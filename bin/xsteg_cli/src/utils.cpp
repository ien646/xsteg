#include "utils.hpp"

#include <cctype>

using namespace xsteg;


pixel_availability parse_px_availability_bits(const std::string& bits_str)
{
    pixel_availability result;

    if(bits_str[0] == '_' || bits_str[0] == '-') { result.r = -1; }
    else { result.r = std::stoi(std::string(1, bits_str[0])); }

    if(bits_str[1] == '_' || bits_str[1] == '-') { result.g = -1; }
    else { result.g = std::stoi(std::string(1, bits_str[1])); }
    
    if(bits_str[2] == '_' || bits_str[2] == '-') { result.b = -1; }
    else { result.b = std::stoi(std::string(1, bits_str[2])); }

    if(bits_str[3] == '_' || bits_str[3] == '-') { result.a = -1; }
    else { result.a = std::stoi(std::string(1, bits_str[3])); }
    
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