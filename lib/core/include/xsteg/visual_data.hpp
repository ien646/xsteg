#pragma once

#include <xsteg/image.hpp>

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

    extern std::unique_ptr<image> generate_visual_data_map(
        const image* imgptr, 
        visual_data_type type);
}
