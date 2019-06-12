#pragma once

#include <vector>

namespace xsteg
{
    extern std::vector<bool> get_last_bits(uint8_t byte, size_t bits);

    extern void set_last_bits(uint8_t* byteptr, const std::vector<bool>& bits);
    
    extern bool get_bit(uint8_t byte, size_t idx);
}