#pragma once

#include <xsteg/image.hpp>
#include <xsteg/visual_data.hpp>
#include <xsteg/pixel_availability.hpp>

#include <cinttypes>
#include <map>
#include <vector>

namespace xsteg
{
    enum class threshold_direction
    {
        UP,
        DOWN
    };    

    struct availability_threshold
    {
        visual_data_type data_type;
        float value;
        threshold_direction direction;
        pixel_availability bits;
    };

    extern std::vector<availability_threshold> parse_thresholds_key(const std::string& key);
    extern std::string generate_thresholds_key(std::vector<availability_threshold>);

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

    private:
        typedef std::map<visual_data_type, std::vector<float>> _vdata_map_map_t;

        void apply_thresholds_st();
        void apply_thresholds_mt(unsigned int thread_count);
        void apply_thresholds_segment(size_t from_px, size_t to_px, const _vdata_map_map_t& vdata_maps);
    };
}