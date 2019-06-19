#include <xsteg/steganographer.hpp>

#include <xsteg/bit_tools.hpp>
#include <xsteg/bit_view.hpp>
#include <xsteg/runtime_settings.hpp>

#include <cassert>
#include <cstring>
#include <iostream>

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

    size_t steganographer::available_space_bits()
    {
        _av_map->apply_thresholds();
        return _av_map->available_data_space();
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
            result[i] = byte;
        }
        return result;
    }

    size_t get_size_from_bytes(const std::vector<uint8_t>& data)
    {
        assert(data.size() >= 8);
        size_t result = 0;
        for(size_t i = 0; i < 8; ++i)
        {
            size_t segment = static_cast<size_t>(data[i]) << ((7 - i) * 8);
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
            size_t mask = bit << (7 - (current_bit%8));

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
            exit(-77);
        }

        bit_view bits(inter_data.data(), bit_len);
        size_t report_counter = 0;

        size_t current_bit = 0;
        auto request_bits = [&](size_t count) -> std::vector<bool>
        {
            auto result = bits.get_bits_at(current_bit, count);
            size_t added_bits = result.size();
            current_bit += added_bits;
            report_counter += added_bits;
            return result;
        };

        size_t cur_pixel = 0;
        auto space_it = space_map.begin();

        std::cout << "Encoding data..." << std::endl;

        size_t report_threshold = std::max(bit_len / 250, (size_t)1);

        // Encode data
        while(current_bit < bit_len)
        {
            if((report_counter > report_threshold) && runtime_settings::verbose)
            {
                report_counter = 0;
                std::cout << "(bits)[" << current_bit << "/" << bit_len << "]\r";
            }
            auto av_bits = *space_it;
            uint8_t* pxptr = _img->data() + (cur_pixel * 4);

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
        std::cout << "(bits)[" << current_bit << "/" << bit_len 
            << "] COMPLETE!" << std::endl;
    }

    std::vector<uint8_t> steganographer::read_data()
    {
        size_t init_data_px_idx = 0;

        std::cout << "Decoding size header..."  << std::endl;
        size_t bit_len = decode_size_header(init_data_px_idx);
        std::cout << "Done! [" << bit_len << "]bits" << std::endl;

        std::vector<bool> read_bits;
        read_bits.reserve(bit_len);

        auto& space_map = _av_map->available_map();

        size_t cur_pixel = 0;
        auto space_it = space_map.begin();

        std::cout << "Reading data..." << std::endl;
        size_t report_counter = 0;
        size_t current_bit = 0;

        auto read_seq = [&](uint8_t px_sgmt, int bit_count)
        {
            auto bits = get_last_bits(px_sgmt, bit_count);
            std::copy(bits.begin(), bits.end(), std::back_inserter(read_bits));
            size_t added_bits = bits.size();
            report_counter += added_bits;
            current_bit += added_bits;
        };

        size_t report_threshold = std::max(bit_len / 1000, (size_t)1);

        while(current_bit < bit_len)
        {
            if((report_counter > report_threshold) && runtime_settings::verbose)
            {
                report_counter = 0;
                std::cout << "(bits)[" << current_bit << "/" << bit_len << "]\r";
            }
            auto av_bits = *space_it;
            uint8_t* pxptr = _img->data() + (cur_pixel * 4);

            if(!av_bits.is_zero())
            {
                if(av_bits.r)
                {
                    read_seq(*(pxptr + 0), av_bits.r);
                }
                if(av_bits.g)
                { 
                    read_seq(*(pxptr + 1), av_bits.g);
                }
                if(av_bits.b)
                { 
                    read_seq(*(pxptr + 2), av_bits.b);
                }
                if(av_bits.a)
                { 
                    read_seq(*(pxptr + 3), av_bits.a);
                }
            }

            ++space_it;
            ++cur_pixel;
        }

        std::cout << "(bits)[" << current_bit << "/" << bit_len 
            << "] COMPLETE!" << std::endl;

        auto allbytes = get_bytes_from_bits(read_bits, 8);
        size_t byte_count = (bit_len / 8) - 8;
        std::vector<uint8_t> result;
        result.reserve(byte_count);

        std::copy(allbytes.begin() + 8, allbytes.begin() + 8 + byte_count, std::back_inserter(result));
        return result;
    }

    void steganographer::save_to_file(const std::string& fname)
    {
        std::cout << "Saving to file... [" << fname << "]" << std::endl;
        _img->write_to_file(fname);
        std::cout << "Done!" << std::endl;
    }

    size_t steganographer::decode_size_header(size_t& skipped_pixels)
    {        
        _av_map->apply_thresholds();
        auto& space_map = _av_map->available_map();
        auto space_it = space_map.begin();
        
        std::vector<bool> sz_bits;
        sz_bits.reserve(64);

        size_t cur_pixel = 0;

        // Decode size header
        while(sz_bits.size() < 64)
        {
            auto av_bits = *space_it;
            uint8_t* pxptr = _img->data() + (cur_pixel * 4);

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

        skipped_pixels = cur_pixel;

        sz_bits.resize(64, 0x00u); // clamp to 64 bits
        std::vector<uint8_t> sz_bytes = get_size_bytes_from_bits(sz_bits);
        return get_size_from_bytes(sz_bytes);
    }
}