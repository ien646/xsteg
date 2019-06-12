#include <xsteg/bit_view.hpp>
#include <cassert>

namespace xsteg
{
    bit_view::bit_view(uint8_t* data_ptr, size_t len)
    {
        _data_ptr = data_ptr;
        _len = len;
    }

    bool bit_view::operator[](size_t index) const
    {
        assert(index < (_len * 8));

        size_t byte_idx = index / 8;
        size_t bit_idx = index % 8;
        uint8_t mask = (uint8_t)1 << bit_idx;
        uint8_t byte = _data_ptr[byte_idx];

        return (byte | mask) == byte;
    }

    size_t bit_view::size() const
    {
        return _len * 8;
    }

    std::vector<bool> bit_view::get_bits_at(size_t index, size_t count) const
    {
        std::vector<bool> result;
        for(size_t i = 0; i < count; ++i)
        {
            result.push_back(this->operator[](index++));
        }
        return result;
    }
}