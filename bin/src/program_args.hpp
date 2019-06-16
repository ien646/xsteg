#pragma once

#include <xsteg/availability_map.hpp>

#include <vector>
#include <string>

enum class encode_mode
{
    NOT_SET,
    ENCODE,
    DECODE,
    DIFF_MAP,
    VDATA_MAPS
};

struct main_args
{
    encode_mode mode = encode_mode::NOT_SET;
    std::string input_img;
    std::string output_img;
    std::vector<xsteg::availability_threshold> thresholds;
    std::vector<uint8_t> data;
    bool output_std = false;
    std::string output_file;
};

extern const std::map<std::string, xsteg::visual_data_type> visual_data_type_name_map;
extern const std::map<std::string, xsteg::threshold_direction> threshold_direction_name_map;
extern main_args parse_main_args(int argc, char** argv);