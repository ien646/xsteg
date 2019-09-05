#pragma once

#include <xsteg/availability_map.hpp>
#include <xsteg/image.hpp>
#include <xsteg/visual_data.hpp>

#include <memory>
#include <string>

namespace xsteg
{
    class steganographer
    {
    private:
        std::unique_ptr<image> _img;
        std::unique_ptr<availability_map> _av_map;

    public:
        explicit steganographer(const std::string& fname);

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

        size_t available_space_bits();

    private:
        size_t decode_size_header(size_t& skipped_px);
    };
}