#include <xsteg/availability_map.hpp>

#include <xsteg/runtime_settings.hpp>
#include <xsteg/synced_print.hpp>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <numeric>

namespace xsteg
{
    availability_map::availability_map(const image* imgptr)
    {
        _img = imgptr;
        _map.resize(_img->pixel_count(), pixel_availability(0, 0, 0, 0));
    }

    void availability_map::add_threshold(
        visual_data_type type, 
        threshold_direction dir, 
        float val, 
        pixel_availability bits)
    {
        _modified = true;
        availability_threshold thresh;
        thresh.data_type = type;
        thresh.direction = dir;
        thresh.value = val;
        thresh.bits = bits;
        _thresholds.push_back(thresh);

        _max_threshold_bits.a = 
            std::max(_max_threshold_bits.a, thresh.bits.a);

        _max_threshold_bits.r = 
            std::max(_max_threshold_bits.r, thresh.bits.r);

        _max_threshold_bits.g = 
            std::max(_max_threshold_bits.g, thresh.bits.g);

        _max_threshold_bits.b = 
            std::max(_max_threshold_bits.b, thresh.bits.b);
    }

    const std::vector<pixel_availability>& availability_map::available_map() const
    {
        return _map;
    }

    const image* availability_map::image_ptr() const
    {
        return _img;
    }

    void availability_map::apply_thresholds()
    {
        if(!_modified) { return; }
        _modified = false;

        static const unsigned int max_threads = std::thread::hardware_concurrency();
        if(runtime_settings::multithreaded && (max_threads > 1))
        {
            apply_thresholds_mt(max_threads);
        }
        else
        {
            apply_thresholds_st();
        }
    }

    void print_apply_thres_sgmt_prog(size_t th_idx, size_t px_idx, size_t from_px, size_t to_px)
    {
        int64_t current_px_seg_idx = (to_px - from_px) - (to_px - px_idx);
        std::stringstream sstr;
        sstr << "THRES[" << th_idx << "] "
             << "SEG["<< from_px << " ~ " << to_px <<"] "
             << "PX[" << current_px_seg_idx << "/" << (to_px - from_px) << "]";
        synced_print(sstr.str(), true);
    }

    void availability_map::apply_thresholds_segment(
        size_t from_px,
        size_t to_px,
        const _vdata_map_map_t& vdata_maps)
    {
        for(size_t thi = 0; thi < _thresholds.size(); ++thi)
        {
            auto& thres = _thresholds[thi];
            const size_t report_threshold_px = 400000 + std::abs(rand() % 100000); // report progress every x pixels
            const std::vector<float>& vdata = vdata_maps.at(thres.data_type);
            for(size_t pxi = from_px; pxi < to_px; ++pxi)
            {
                float pxv = vdata[pxi];
                bool cond = (thres.direction == threshold_direction::UP)
                            ? pxv >= thres.value
                            : pxv <= thres.value;

                if(cond)
                {
                    if(thres.bits.r >= 0)
                        { _map[pxi].r = thres.bits.r; }
                    if(thres.bits.g >= 0)
                        { _map[pxi].g = thres.bits.g; }
                    if(thres.bits.b >= 0)
                        { _map[pxi].b = thres.bits.b; }
                    if(thres.bits.a >= 0)
                        { _map[pxi].a = thres.bits.a; }
                }
                if(runtime_settings::verbose)
                {
                    size_t current_px_seg_idx = (to_px - pxi) - from_px;
                    if(current_px_seg_idx % report_threshold_px == 0)
                    {
                        print_apply_thres_sgmt_prog(thi, pxi, from_px, to_px);
                    }
                }
            }
            if(runtime_settings::verbose)
            {
                print_apply_thres_sgmt_prog(thi, to_px, from_px, to_px);
            }
        }
    }

    void availability_map::apply_thresholds_st()
    {
        _vdata_map_map_t vdata_maps;
        for(auto& thres : _thresholds)
        {
            if(!vdata_maps.count(thres.data_type))
            {
                std::vector<float> vdata_map = 
                    get_visual_data_map(
                        _img, 
                        thres.data_type, 
                        thres.bits
                    );
                
                vdata_maps.emplace(thres.data_type, std::move(vdata_map));
            }
        }
        apply_thresholds_segment(0, _img->pixel_count() - 1, vdata_maps);
    }

    void availability_map::apply_thresholds_mt(unsigned int thread_count)
    {
        _vdata_map_map_t vdata_maps;
        for(auto& thres : _thresholds)
        {
            if(!vdata_maps.count(thres.data_type))
            {
                std::vector<float> vdata_map = 
                    get_visual_data_map(
                        _img, 
                        thres.data_type, 
                        thres.bits
                    );
                
                vdata_maps.emplace(thres.data_type, std::move(vdata_map));
            }
        }

        const size_t pixel_count = _img->pixel_count();
        const size_t thread_segment_size = pixel_count / thread_count;

        std::vector<std::thread> threads;
        for(auto i = 0u; i < thread_count - 1; ++i)
        {
            threads.push_back(
                std::thread(
                    &availability_map::apply_thresholds_segment,
                    this,
                    i * thread_segment_size,
                    ((i + 1) * thread_segment_size) - 1,
                    vdata_maps
                )
            );
        }
        threads.push_back(
            std::thread(
                &availability_map::apply_thresholds_segment,
                this,
                (thread_count - 1) * thread_segment_size,
                thread_count * thread_segment_size + (pixel_count % thread_segment_size),
                vdata_maps
            )
        );

        for(auto& th : threads)
        {
            th.join();
        }
    }

    std::map<visual_data_type, char> type_designators = {
        { visual_data_type::ALPHA, '3' },
        { visual_data_type::AVERAGE_VALUE_RGB, 'V' },
        { visual_data_type::AVERAGE_VALUE_RGBA, 'W' }, 
        { visual_data_type::COLOR_BLUE, '2' },
        { visual_data_type::COLOR_GREEN, '1' }, 
        { visual_data_type::COLOR_RED, '0' },
        { visual_data_type::LUMINANCE, 'L' },
        { visual_data_type::SATURATION, 'S' }
    };

    std::map<char, visual_data_type> type_designators_rev = {
        { '3', visual_data_type::ALPHA  },
        { 'V', visual_data_type::AVERAGE_VALUE_RGB  },
        { 'W', visual_data_type::AVERAGE_VALUE_RGBA  }, 
        { '2', visual_data_type::COLOR_BLUE },
        { '1', visual_data_type::COLOR_GREEN }, 
        { '0', visual_data_type::COLOR_RED },
        { 'L', visual_data_type::LUMINANCE },
        { 'S', visual_data_type::SATURATION }
    };

    std::string bits_ov_to_string(pixel_availability& bits)
    {
        std::stringstream ss;
        if(bits.r == -1) { ss << '_'; } else { ss << bits.r; }
        if(bits.g == -1) { ss << '_'; } else { ss << bits.g; }
        if(bits.b == -1) { ss << '_'; } else { ss << bits.b; }
        if(bits.a == -1) { ss << '_'; } else { ss << bits.a; }
        return ss.str();
    }

    size_t availability_map::available_data_space()
    {
        return std::accumulate<decltype(_map)::const_iterator, size_t>(
            _map.begin(),
            _map.end(),
            0, 
            [](size_t accum, const pixel_availability& av) -> size_t
            {
                return accum + (av.r + av.g + av.b + av.a);
            }
        );
    }

    const char TYPE_DESIGNATOR = '&';
    const char DIRECTION_DESIGNATOR = '>';
    const char BITS_OV_DESIGNATOR = '*';
    const char VALUE_DESIGNATOR = '+';

    std::string availability_map::generate_key()
    {
        std::stringstream ss;
        for(auto& threshold : _thresholds)
        {
            bool up = threshold.direction == threshold_direction::UP;
            ss  << TYPE_DESIGNATOR
                << type_designators[threshold.data_type] 
                << DIRECTION_DESIGNATOR
                << (up ? 'A' : 'V')
                << BITS_OV_DESIGNATOR
                << bits_ov_to_string(threshold.bits)
                << VALUE_DESIGNATOR
                << threshold.value;
        }
        return ss.str();
    }

    std::vector<std::string> str_split(std::string_view strv, char delim)
    {
        if (strv.size() <= 1)
        {
            std::vector<std::string> result;
            result.push_back(std::string(strv));
            return result;
        }

        std::vector<std::string> result;
        auto beg_it = strv.begin();
        auto cur_it = strv.begin();

        while (cur_it != strv.end())
        {
            if (*cur_it == delim)
            {
                result.push_back(std::string(beg_it, cur_it));
                beg_it = ++cur_it;
            }
            else
            {
                cur_it++;
            }
        }

        result.push_back(std::string(beg_it, cur_it));
        return result;
    }

    void availability_map::restore_from_key(const std::string& key)
    {
        auto thresholds = parse_key(key);
        std::move(thresholds.begin(), thresholds.end(), std::back_inserter(_thresholds));
        _modified = true;
    }

    const pixel_availability& availability_map::max_threshold_bits()
    {
        return _max_threshold_bits;
    }

    std::vector<availability_threshold> availability_map::parse_key(const std::string& key)
    {
        std::vector<availability_threshold> result;

        std::vector<std::string> types = 
            str_split(std::string_view(key), TYPE_DESIGNATOR);

        for(auto& type : types)
        {
            if(type.empty()) { continue; }
            visual_data_type vdt = type_designators_rev.at(type[0]);
            if(type[1] != DIRECTION_DESIGNATOR) { exit(-1); }
            threshold_direction dir = (type[2] == 'A') 
                ? threshold_direction::UP 
                : threshold_direction::DOWN;

            if(type[3] != BITS_OV_DESIGNATOR) { exit(-2); }

            int br, bg, bb, ba;
            if(type[4] == '_'){ br = -1; } else { br = std::stoi(std::string(1, type[4])); }
            if(type[5] == '_'){ bg = -1; } else { bg = std::stoi(std::string(1, type[5])); }
            if(type[6] == '_'){ bb = -1; } else { bb = std::stoi(std::string(1, type[6])); }
            if(type[7] == '_'){ ba = -1; } else { ba = std::stoi(std::string(1, type[7])); }

            if(type[8] != VALUE_DESIGNATOR) { exit(-3); }
            std::string fstr = type.substr(9);
            float val = std::stof(fstr);
            availability_threshold th;
            th.data_type = vdt;
            th.direction = dir;
            th.value = val;
            th.bits = pixel_availability(br, bg, bb, ba);
            result.push_back(th);
        }
        return result;
    }
}