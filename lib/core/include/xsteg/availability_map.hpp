#pragma once

#include <xsteg/image.hpp>
#include <xsteg/visual_data.hpp>

#include <vector>
#include <map>
#include <cinttypes>

namespace xsteg
{
    enum class threshold_direction
    {
        UP,
        DOWN
    };    
    
    struct pixel_availability
    {
        int r = 0;
        int g = 0;
        int b = 0;
        int a = 0;

        constexpr pixel_availability() noexcept = default;
        constexpr pixel_availability(int rv, int gv, int bv, int av) noexcept
        {
            r = rv;
            g = gv;
            b = bv;
            a = av;
        }

        constexpr bool is_zero() const
        {
            return r == 0 && g == 0 && b == 0 && a == 0;
        }
    };

    struct availability_threshold
    {
        visual_data_type data_type;
        float value;
        threshold_direction direction;
        pixel_availability bits;
    };

    class availability_map
    {
    private:
        const image* _img = nullptr;
        std::vector<pixel_availability> _map;
        std::vector<availability_threshold> _thresholds;
        pixel_availability _max_threshold_bits;
        bool _modified = true;

    public:
        availability_map(const image* imgptr);

        void add_threshold(
            visual_data_type type, 
            threshold_direction dir, 
            float val, 
            pixel_availability bits);

        void apply_thresholds();

        const std::vector<pixel_availability>& available_map() const;

        size_t available_data_space();

        const image* image_ptr() const;

        std::string generate_key();
        void restore_from_key(const std::string& key);

        const pixel_availability& max_threshold_bits();

        static std::vector<availability_threshold> parse_key(const std::string& key);
    };
}