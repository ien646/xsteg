#include <xsteg/steganographer.hpp>

#include <xsteg/bit_view.hpp>
#include <xsteg/bit_tools.hpp>

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

    void steganographer::write_data(uint8_t* data, size_t len)
    {
        _av_map->apply_thresholds();
        auto& space_map = _av_map->available_map();
        size_t available_space = _av_map->available_data_space();
        size_t bit_len = (len * 8);

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
            ++cur_pixel;;
        }
    }

    void steganographer::save_to_file(const std::string& fname)
    {
        _img->write_to_file(fname);
    }
}