#pragma once

#include <xsteg/image.hpp>
#include <xsteg/pixel_availability.hpp>
#include <vector>

namespace xsteg
{
    struct pixel_availability;
    
    enum class visual_data_type
    {
        COLOR_RED,
        COLOR_GREEN,
        COLOR_BLUE,
        LUMINANCE,
        SATURATION,
        ALPHA,
        AVERAGE_VALUE_RGBA,
        AVERAGE_VALUE_RGB
    };

    extern float get_visual_data(
        const uint8_t* px, 
        visual_data_type type,
        pixel_availability truncate_bits);

    extern std::vector<float> get_visual_data_map(
        const image* img, 
        visual_data_type type,
        pixel_availability truncate_bits);

    extern image generate_visual_data_image(
        const image* imgptr, 
        visual_data_type type,
        pixel_availability truncate_bits = pixel_availability(0, 0, 0, 0));

    extern image generate_visual_data_diff_image(
        const image* imgptr, 
        visual_data_type type,
        float val_diff,
        pixel_availability truncate_bits = pixel_availability(0, 0, 0, 0));
}
