#include <xsteg/bit_tools.hpp>
#include <cassert>

namespace xsteg
{
    std::vector<bool> get_last_bits(uint8_t byte, size_t bits)
    {
        assert(bits <= 8 && bits >= 0);
        std::vector<bool> result;
        while(bits > 0)
        {
            uint8_t mask = (uint8_t)1 << bits;
            result.push_back((byte | mask) == byte);
            --bits;
        }
        return result;
    }

    void set_last_bits(uint8_t* byteptr, const std::vector<bool>& bits)
    {
        assert(bits.size() <= 8);
        for(size_t i = 0; i < bits.size(); ++i)
        {
            uint8_t bit = (uint8_t)(bits[i] ? 1 : 0);
            uint8_t mask = bit << (bits.size() - 1 - i);
            *byteptr |= mask;
        }
    }

    bool get_bit(uint8_t byte, size_t idx)
    {
        assert(idx < 7 && idx >= 0);
        uint8_t mask = (uint8_t)1 << idx;
        return (byte | mask) == byte;
    }
}