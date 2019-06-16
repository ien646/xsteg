#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <cinttypes>
#include <xsteg/availability_map.hpp>

extern void str_toupper(std::string& str);
extern xsteg::pixel_availability parse_px_availability_bits(const std::string& bits_str);
extern std::vector<uint8_t> str_to_datavec(const std::string& str);