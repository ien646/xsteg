#include <xsteg/bit_tools.hpp>
#include <cassert>

namespace xsteg
{
    std::vector<bool> get_last_bits(uint8_t byte, size_t bits)
    {
        assert(bits <= 8 && bits > 0);
        std::vector<bool> result;
        while(bits > 0)
        {
            uint8_t mask = (uint8_t)1 << (bits - 1);
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
            uint8_t clear_mask = ~(1 << (bits.size() - 1 - i));
            *byteptr &= clear_mask;
            *byteptr |= mask;
        }
    }

    bool get_bit(uint8_t byte, size_t idx)
    {
        assert(idx < 7 && idx >= 0);
        uint8_t mask = (uint8_t)1 << idx;
        return (byte | mask) == byte;
    }

    std::vector<uint8_t> get_bytes_from_bits(const std::vector<bool>& data, size_t offset_bytes)
    {
        assert((offset_bytes * 8) < data.size());
        std::vector<uint8_t> result;   
        result.resize((data.size() - (offset_bytes * 8)) * 8, 0x00u);

        size_t current_bit = offset_bytes * 8;
        while(current_bit < data.size())
        {
            size_t bit_offset = current_bit % 8;
            size_t byte_idx = current_bit / 8;

            uint8_t bit = (uint8_t)(data[current_bit] ? 1 : 0);
            
            uint8_t mask = (uint8_t)bit << (7 - bit_offset);
            result[byte_idx] |= mask;

            ++current_bit;
        }

        return result;
    }
}