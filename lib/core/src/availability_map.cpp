#include <xsteg/availability_map.hpp>

#include <xsteg/runtime_settings.hpp>
#include <algorithm>
#include <sstream>
#include <map>
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <chrono>

namespace xsteg
{
    availability_map::availability_map(const image* imgptr)
    {
        _img = imgptr;
        _map.resize(_img->pixel_count());
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
        static unsigned int max_threads = std::thread::hardware_concurrency();
        if(!_modified) { return; }
        
        int th_i = 0;
        for(auto& thres : _thresholds)
        {
            ++th_i;
            std::cout << "Applying threshold [" << th_i << "/" 
                << _thresholds.size() << "]" << std::endl;

            const size_t pixel_count =_img->pixel_count();
            const size_t report_threshold = std::max(pixel_count / 500, (size_t)5);
            size_t report_th_accum = 0;

            std::mutex report_mex;

            auto thread_segment = [&](size_t from, size_t to)
            {
                size_t report_accum = 0;
                for(size_t i = from; i <= to; ++i)
                {
                    float pxv = get_visual_data(
                        _img->cpixel_at_idx(i), 
                        thres.data_type, 
                        thres.bits
                    );
                    bool cond = (thres.direction == threshold_direction::UP)
                                ? pxv >= thres.value
                                : pxv <= thres.value;

                    if(pxv >= thres.value)
                    {
                        _map[i].r = thres.bits.r;
                        _map[i].g = thres.bits.g;
                        _map[i].b = thres.bits.b;
                        _map[i].a = thres.bits.a;
                    }
                    if((report_accum++ > report_threshold) == 0)   
                    {
                        if(report_mex.try_lock())
                        {
                            report_th_accum += report_accum;
                            report_accum = 0;
                            report_mex.unlock();
                        }
                    }
                }
            };

            size_t thread_segment_size = pixel_count / max_threads;

            std::vector<std::thread> threads;
            for(size_t i = 0; i < max_threads - 1; ++i)
            {
                size_t from = (i * thread_segment_size);
                size_t to = from + thread_segment_size;
                threads.push_back(
                    std::thread(thread_segment, from, to)
                );
            }

            bool end_report = false;
            auto report_thread = std::thread([&]()
            {
                if(runtime_settings::verbose)
                {
                    while(!end_report)
                    {
                        std::cout << "(pixels)[" << report_th_accum << "/" << pixel_count << "]\r";
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    }
                }
            });

            for(auto& th : threads) { th.join(); }
            end_report = true;
            report_thread.join();
            
            std::cout << "> (pixels)[" << _img->pixel_count() << "/" 
                << _img->pixel_count() << "]\r";
            std::cout << std::endl;
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
        ss << bits.r;
        ss << bits.g;
        ss << bits.b;
        ss << bits.a;
        return ss.str();
    }

    size_t availability_map::available_data_space()
    {
        size_t accum = 0;
        for(auto& pxav : _map)
        {
            accum += (pxav.r + pxav.g + pxav.b + pxav.a);
        }
        return accum;
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
            uint8_t br = std::stoi(std::string(1, type[4]));
            uint8_t bg = std::stoi(std::string(1, type[5]));
            uint8_t bb = std::stoi(std::string(1, type[6]));
            uint8_t ba = std::stoi(std::string(1, type[7]));
            if(type[8] != VALUE_DESIGNATOR) { exit(-3); }
            std::string fstr = type.substr(9);
            float val = std::stof(fstr);
            add_threshold(vdt, dir, val, pixel_availability(br, bg, bb, ba));
        }
    }

    const pixel_availability& availability_map::max_threshold_bits()
    {
        return _max_threshold_bits;
    }
}