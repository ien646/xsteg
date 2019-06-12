#include <xsteg/steganographer.hpp>

#include <xsteg/bit_view.hpp>
#include <xsteg/bit_tools.hpp>

#include <iostream>
#include <cassert>

namespace xsteg
{
    steganographer::steganographer(const std::string& fname)
    {
        _img = std::make_unique<image>(fname);
        _av_map = std::make_unique<availability_map>(_img.get());
    }

    void steganographer::add_threshold(
        visual_data_type type, 
        threshold_direction dir, 
        float val, 
        pixel_availability bits)
    {
        _av_map->add_threshold(type, dir, val, bits);
    }

    std::string steganographer::get_key()
    {
        return _av_map->generate_key();
    }

    void steganographer::restore_key(const std::string& key)
    {
        _av_map->restore_from_key(key);
        _av_map->apply_thresholds();
    }

    std::vector<uint8_t> get_bytes_for_size(size_t sz)
    {
        static_assert(sizeof(size_t) == 8, "Size of size_t is not 8 bytes. Platform not supported.");

        std::vector<uint8_t> result;
        result.resize(8);        
        for(size_t i = 0; i < 8; ++i)
        {
            uint8_t byte = static_cast<uint8_t>(sz >> ((7 - i) * 8));
            result.push_back(byte);
        }
        return result;
    }

    size_t get_size_from_bytes(const std::vector<uint8_t>& data)
    {
        assert(data.size() >= 8);
        size_t result = 0;
        for(size_t i = 0; i < 8; ++i)
        {
            size_t segment = static_cast<size_t>(data[i]) << (7 - i);
            result |= segment;
        }
        return result;
    }

    std::vector<uint8_t> get_size_bytes_from_bits(const std::vector<bool>& data)
    {
        std::vector<uint8_t> result;
        result.resize(8, 0x00u);
        size_t current_bit = 0;

        while (current_bit < 64)
        {
            size_t bit = static_cast<size_t>(data[current_bit] ? 1 : 0);
            size_t mask = bit << (7 - current_bit);

            size_t byte_idx = current_bit / 8;
            result[byte_idx] |= mask;

            ++current_bit;
        }
        return result;
    }

    void steganographer::write_data(uint8_t* data, size_t len)
    {
        _av_map->apply_thresholds();

        size_t bit_len = ((len * 8) + 64);
        std::vector<uint8_t> size_data = get_bytes_for_size(bit_len);

        std::vector<uint8_t> inter_data;
        inter_data.resize(len + 8, 0x00u);

        std::memcpy(inter_data.data(), size_data.data(), 8);        
        std::memcpy(inter_data.data() + 8, data, len);
        
        auto& space_map = _av_map->available_map();
        size_t available_space = _av_map->available_data_space();

        if(available_space < bit_len)
        {
            std::cerr << "Not enough available space to encode data!" << std::endl;
            std::cerr 
                << "Requested [" 
                << bit_len
                << "]b to encode, while "
                << "maximum allowed space is [" 
                << available_space 
                << "]b for the given destination and thresholds."
                << std::endl;
        }

        bit_view bits(data, len);
        
        size_t current_bit = 0;
        auto request_bits = [&](size_t count) -> std::vector<bool>
        {
            auto result = bits.get_bits_at(current_bit, count);
            current_bit += result.size();
            return result;
        };        

        size_t cur_pixel = 0;
        auto space_it = space_map.begin();

        // Encode data
        while(current_bit < bit_len)
        {
            auto av_bits = *space_it;
            uint8_t* pxptr = _img->data() + cur_pixel;

            if(!av_bits.is_zero())
            {
                if(av_bits.r) { set_last_bits(pxptr + 0, request_bits(av_bits.r)); }
                if(av_bits.g) { set_last_bits(pxptr + 1, request_bits(av_bits.g)); }
                if(av_bits.b) { set_last_bits(pxptr + 2, request_bits(av_bits.b)); }
                if(av_bits.a) { set_last_bits(pxptr + 3, request_bits(av_bits.a)); }
            }

            ++space_it;
            ++cur_pixel;
        }
    }

    std::vector<uint8_t> steganographer::read_data()
    {
        size_t bit_len = decode_size_header();

        std::vector<bool> read_bits;
        read_bits.reserve(bit_len);

        auto& space_map = _av_map->available_map();

        size_t cur_pixel = 0;
        auto space_it = space_map.begin();

        size_t current_bit = 0;
        while(current_bit < bit_len)
        {
            auto av_bits = *space_it;
            uint8_t* pxptr = _img->data() + cur_pixel;

            if(!av_bits.is_zero())
            {
                if(av_bits.r)
                { 
                    auto bits = get_last_bits(*(pxptr + 0), av_bits.r);
                    std::copy(bits.begin(), bits.end(), std::back_inserter(read_bits));
                }
                if(av_bits.g)
                { 
                    auto bits = get_last_bits(*(pxptr + 1), av_bits.g);
                    std::copy(bits.begin(), bits.end(), std::back_inserter(read_bits));
                }
                if(av_bits.b)
                { 
                    auto bits = get_last_bits(*(pxptr + 2), av_bits.b);
                    std::copy(bits.begin(), bits.end(), std::back_inserter(read_bits));
                }
                if(av_bits.a)
                { 
                    auto bits = get_last_bits(*(pxptr + 3), av_bits.a);
                    std::copy(bits.begin(), bits.end(), std::back_inserter(read_bits));
                }
            }

            ++space_it;
            ++cur_pixel;
        }

        return get_bytes_from_bits(read_bits, 8);
    }

    void steganographer::save_to_file(const std::string& fname)
    {
        _img->write_to_file(fname);        
    }

    size_t steganographer::decode_size_header()
    {
        auto& space_map = _av_map->available_map();
        auto space_it = space_map.begin();
        
        std::vector<bool> sz_bits;
        sz_bits.reserve(64);

        size_t cur_pixel = 0;

        // Decode size header
        while(sz_bits.size() < 64)
        {
            auto av_bits = *space_it;
            uint8_t* pxptr = _img->data() + cur_pixel;

            if(!av_bits.is_zero())
            {
                if(av_bits.r) 
                { 
                    auto bits = get_last_bits(*(pxptr + 0), av_bits.r);
                    std::copy(bits.begin(), bits.end(), std::back_inserter(sz_bits));
                }
                if(av_bits.g) 
                { 
                    auto bits = get_last_bits(*(pxptr + 1), av_bits.g); 
                    std::copy(bits.begin(), bits.end(), std::back_inserter(sz_bits));
                }
                if(av_bits.b) 
                { 
                    auto bits = get_last_bits(*(pxptr + 2), av_bits.b); 
                    std::copy(bits.begin(), bits.end(), std::back_inserter(sz_bits));
                }
                if(av_bits.a) 
                { 
                    auto bits = get_last_bits(*(pxptr + 3), av_bits.a); 
                    std::copy(bits.begin(), bits.end(), std::back_inserter(sz_bits));
                }
            }

            ++space_it;
            ++cur_pixel;
        }

        sz_bits.resize(64, 0x00u); // clamp to 64 bits
        std::vector<uint8_t> sz_bytes = get_size_bytes_from_bits(sz_bits);
        return get_size_from_bytes(sz_bytes);
    }
}