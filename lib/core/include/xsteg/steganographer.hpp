#pragma once

#include <xsteg/image.hpp>
#include <xsteg/visual_data.hpp>
#include <xsteg/availability_map.hpp>
#include <string>
#include <memory>

namespace xsteg
{
    class steganographer
    {
    private:
        std::unique_ptr<image> _img;
        std::unique_ptr<availability_map> _av_map;

    public:
        steganographer(const std::string& fname);

        void add_threshold(
            visual_data_type type, 
            threshold_direction dir, 
            float val, 
            pixel_availability bits);

        std::string get_key();
        void restore_key(const std::string& key);

        void write_data(uint8_t* data, size_t len);
        std::vector<uint8_t> read_data();
        
        void save_to_file(const std::string& fname);

    private:
        size_t decode_size_header();
    };
}