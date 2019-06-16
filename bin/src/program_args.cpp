#include "program_args.hpp"

#include <fstream>
#include <xsteg/runtime_settings.hpp>

#include "utils.hpp"

using namespace xsteg;
using std::string;
using std::map;

const map<string, visual_data_type> visual_data_type_name_map = 
{
    { "ALPHA",              visual_data_type::ALPHA },
    { "AVERAGE_VALUE_RGB",  visual_data_type::AVERAGE_VALUE_RGB },
    { "AVERAGE_VALUE_RGBA", visual_data_type::AVERAGE_VALUE_RGBA },
    { "COLOR_BLUE",         visual_data_type::COLOR_BLUE },
    { "COLOR_GREEN",        visual_data_type::COLOR_GREEN },
    { "COLOR_RED",          visual_data_type::COLOR_RED },
    { "LUMINANCE",          visual_data_type::LUMINANCE },
    { "SATURATION",         visual_data_type::SATURATION },
};

const map<string, threshold_direction> threshold_direction_name_map =
{
    { "UP", threshold_direction::UP },
    { "DOWN", threshold_direction::DOWN}
};

main_args parse_main_args(int argc, char** argv)
{
    size_t current_arg = 1;
    main_args result;
    auto next_arg = [&]() -> std::string { return std::string(argv[current_arg++]); };

    while (current_arg < argc)
    {
        std::string arg(next_arg());
        if(arg == "-ii")        { result.input_img = next_arg(); }
        else if(arg == "-oi")   { result.output_img = next_arg(); }
        else if(arg == "-e")    { result.mode = encode_mode::ENCODE; }
        else if(arg == "-d")    { result.mode = encode_mode::DECODE; }
        else if(arg == "-o")    { result.output_std = true; }
        else if(arg == "-of")   { result.output_file = next_arg(); }
        else if(arg == "-x")    { result.data = str_to_datavec(next_arg()); }
        else if(arg == "-m")    { result.mode = encode_mode::DIFF_MAP; }
        else if(arg == "-vd")   { result.mode = encode_mode::VDATA_MAPS; }
        else if(arg == "-v")    { runtime_settings::verbose = true; }
        else if(arg == "-t")
        {
            std::string type = next_arg();
            std::string dir  = next_arg();
            std::string val  = next_arg();
            std::string bits = next_arg();
            str_toupper(type);
            str_toupper(dir);

            availability_threshold threshold;
            threshold.data_type = visual_data_type_name_map.at(type);
            threshold.direction = threshold_direction_name_map.at(dir);
            threshold.value = std::stof(val);
            threshold.bits = parse_px_availability_bits(bits);
            result.thresholds.push_back(threshold);
        }        
        else if(arg == "-df")
        {
            std::string datafile = next_arg();
            std::ifstream ifs(datafile, std::ios::binary | std::ios::ate);
            size_t fsize = static_cast<size_t>(ifs.tellg());

            result.data.clear();
            result.data.resize(fsize, 0x00u);
            
            char* data_ptr = reinterpret_cast<char*>(result.data.data());

            ifs.seekg(0, std::ios::beg);
            ifs.read(data_ptr, fsize);
            ifs.close();
        }        
    }
    return result;
}