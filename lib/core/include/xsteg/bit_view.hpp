#pragma once

#include <cinttypes>
#include <vector>

namespace xsteg
{
    class bit_view
    {
    private:
        uint8_t* _data_ptr = nullptr;
        size_t _len = 0;

    public:
        bit_view(uint8_t* data_ptr, size_t len);
        
        bool operator[](size_t index) const;

        std::vector<bool> get_bits_at(size_t index, size_t count) const;

        size_t size() const;
    };
}