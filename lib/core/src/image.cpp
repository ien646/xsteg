#include <xsteg/image.hpp>
#include <xsteg/availability_map.hpp>

#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"

namespace xsteg
{
    image::image(int width, int height)
    {
        _loaded_stbi = false;
        _width = width;
        _height = height;
        _channels = 4;

        int sz = (width * height) * 4;
        _data = new uint8_t[sz];
        std::fill(_data, _data + sz, 0xFF);
    }

    image::image(const std::string& fname)
    {
        read_from_file(fname);
    }

    image::~image()
    {
        if(_loaded_stbi)
        {
            stbi_image_free(_data);
        }
        else
        {
            delete[](_data);
        }
    }

    image::image(image&& mv_src) noexcept
    {
        _data = mv_src._data;
        _width = mv_src._width;
        _height = mv_src._height;
        _channels = mv_src._channels;
        _loaded_stbi = mv_src._loaded_stbi;

        mv_src._data = nullptr;
        mv_src._channels = -1;
        mv_src._height = -1;
        mv_src._loaded_stbi = false;
        mv_src._width = -1;
    }

    void image::operator=(image&& mv_src) noexcept
    {
        _data = mv_src._data;
        _width = mv_src._width;
        _height = mv_src._height;
        _channels = mv_src._channels;
        _loaded_stbi = mv_src._loaded_stbi;

        mv_src._data = nullptr;
        mv_src._channels = -1;
        mv_src._height = -1;
        mv_src._loaded_stbi = false;
        mv_src._width = -1;
    }

    image image::create_copy()
    {
        image result(_width, _height);
        assert(result._width == this->_width);
        assert(result._height == this->_height);
        assert(result._loaded_stbi == false);
        std::memcpy(result._data, this->_data, pixel_count() * 4);
        return result;
    }

    image image::create_resized_copy_absolute(int px_width, int px_height)
    {
        image result(px_width, px_height);
        stbir_resize_uint8(
            _data, 
            _width, 
            _height, 
            0, 
            result.data(), 
            result.width(), 
            result.height(), 
            0,
            4 /* RGBA */
        );
        return result;
    }

    image image::create_resized_copy_proportional(float percentage_w, float percentage_h)
    {
        int pxw = static_cast<int>(_width * (percentage_h / 100));
        int pxh = static_cast<int>(_height * (percentage_w / 100));
        return create_resized_copy_absolute(pxw, pxh);
    }

    void image::read_from_file(const std::string& fname)
    {
        _loaded_stbi = true;
        _data = stbi_load(
            fname.c_str(), 
            &_width, 
            &_height, 
            &_channels, 
            4
        );
        _channels = 4;
        if(_data == nullptr)
        {
			throw std::invalid_argument(
				std::string("Unable to open image file: [") + fname + "]"
			);
        }
    }

    void image::write_to_file(const std::string& fname, image_save_options opt)
    {
        switch(opt.format)
        {
            case image_format::png:
            {
                write_to_file_png(fname);
            }
            case image_format::jpeg:
            {
                write_to_file_jpeg(fname, opt.jpeg_quality);
            }
        }
    }

    const uint8_t* image::cdata() const
    {
        return _data;
    }

    uint8_t* image::data()
    {
        return _data;
    }

    int image::width() const { return _width; }

    int image::height() const { return _height; }

    int image::channels() const { return _channels; }

    uint8_t* image::pixel_at_idx(size_t idx)
    {
        assert((idx) < pixel_count());
        return _data + (idx * 4);
    }

    const uint8_t* image::cpixel_at_idx(size_t idx) const
    {
        assert((idx) < pixel_count());
        return _data + (idx * 4);
    }

    size_t image::pixel_count() const
    {
        return static_cast<size_t>((size_t)_width * _height);
    }

    void image::truncate_threshold_bits(
        pixel_availability& bits,
        size_t max_truncated_bits)
    {
        uint8_t mask_r = ~((uint8_t)std::pow(2, bits.r)) - 1;
        uint8_t mask_g = ~((uint8_t)std::pow(2, bits.g)) - 1;
        uint8_t mask_b = ~((uint8_t)std::pow(2, bits.b)) - 1;
        uint8_t mask_a = ~((uint8_t)std::pow(2, bits.a)) - 1;

        size_t step_bits = ((size_t)bits.r + bits.g + bits.b + bits.a);
        size_t bit_count_accum = 0;

        for(size_t i = 0; i < pixel_count(); ++i)
        {
            uint8_t* pxptr = _data + (i * 4);
            pxptr[0] &= mask_r;
            pxptr[1] &= mask_g;
            pxptr[2] &= mask_b;
            pxptr[3] &= mask_a;

            bit_count_accum += step_bits;
            if(bit_count_accum >= max_truncated_bits) { break; }
        }
    }

    void image::write_to_file_png(const std::string& fname)
    {
        int result = stbi_write_png(
            fname.c_str(), 
            _width, 
            _height, 
            static_cast<int>(_channels), 
            _data, 
            _width * static_cast<int>(_channels)
        );
        if(result == 0)
        {
			throw std::invalid_argument(
				std::string("Unable to save png image file: [") + fname + "]"
			);
        }
    }

    void image::write_to_file_jpeg(const std::string& fname, int quality)
    {
        int result = stbi_write_jpg(
            fname.c_str(), 
            _width, 
            _height, 
            static_cast<int>(_channels), 
            _data, 
            quality
        );
        if(result == 0)
        {
			throw std::invalid_argument(
				std::string("Unable to save jpeg image file: [") + fname + "]"
			);
        }
    }
}