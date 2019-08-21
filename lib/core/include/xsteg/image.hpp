#pragma once

#include <array>
#include <cinttypes>
#include <string>

namespace xsteg
{
    class image;
    class image_pixel_view;
    class image_reader;
    struct pixel_availability;

    class image
    {
    private:
        uint8_t* _data = nullptr;
        int _width = 0;
        int _height = 0;
        int _channels = 0;
        bool _loaded_stbi = false;

    public:
        image(int width, int height);
        image(const std::string& fname);
        ~image();

        image(const image& cp_src) = delete;
        image(image&& mv_src);

        image create_copy();

        image create_resized_copy_absolute(int px_width, int px_height);
        image create_resized_copy_proportional(float percentage_w, float percentage_h);

        void read_from_file(const std::string& fname);
        void write_to_file(const std::string& fname);

        const uint8_t* cdata() const;
        uint8_t* data();

        int width() const;
        int height() const;
        int channels() const;
        size_t pixel_count() const;

        uint8_t* pixel_at_idx(size_t idx);
        const uint8_t* cpixel_at_idx(size_t idx) const;

        void truncate_threshold_bits(
            pixel_availability&, 
            size_t max_truncated_bits = std::numeric_limits<size_t>::max());
    };
}