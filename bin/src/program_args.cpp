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
    int current_arg = 1;
    main_args result;
    auto next_arg = [&]() -> std::string { return std::string(argv[current_arg++]); };

    while (current_arg < argc)
    {
        std::string arg(next_arg());
        if(arg == "-ii")        { result.input_img = next_arg(); }
        else if(arg == "-oi")   { result.output_img = next_arg(); }
        else if(arg == "-e")    { result.mode = encode_mode::ENCODE; }
        else if(arg == "-d")    { result.mode = encode_mode::DECODE; }
        else if(arg == "-m")    { result.mode = encode_mode::DIFF_MAP; }
        else if(arg == "-vd")   { result.mode = encode_mode::VDATA_MAPS; }
        else if(arg == "-h")    { result.mode = encode_mode::HELP; }
        else if(arg == "-gk")   { result.mode = encode_mode::GENERATE_KEY; }        
        else if(arg == "-o")    { result.output_std = true; }
        else if(arg == "-of")   { result.output_file = next_arg(); }
        else if(arg == "-x")    { result.data = str_to_datavec(next_arg()); }
        else if(arg == "-v")    { runtime_settings::verbose = true; }
        else if(arg == "-t")
        {
            std::string type = next_arg();
            std::string dir  = next_arg();            
            std::string bits = next_arg();
            std::string val  = next_arg();
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
        else if(arg == "-rk")
        {
            result.restore_key = next_arg();
        }
        else if(arg == "-nomt")
        {
            runtime_settings::multithreaded = false;
        }
    }
    return result;
}

const std::string help_text = "\
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
 xsteg - Image Steganography Tool\n\
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
\n\
\n\
Command-Line Tool Usage\n\
========================n\
\n\
\n\
Encoding mode arguments (pick one)\n\
----------------------------------\n\
    '-e':  Encode\n\
    '-d':  Decode\n\
    '-m':  Diff-map\n\
    '-vd': Generate visual-data maps\n\
    '-gk': Generate thresholds key\n\
\n\
Threshold '-t' specification (-t [0](visual_data) [1](direction) [2](orb_mask) [3](value)):\n\
    [0]: Visual data type\n\
        - Available types:\n\
         > COLOR_RED\n\
         > COLOR_GREEN\n\
         > COLOR_BLUE\n\
         > ALPHA\n\
         > AVERAGE_RGB\n\
         > AVERAGE_RGBA\n\
         > SATURATION\n\
         > LUMINANCE\n\
\n\
    [1]: Threshold direction\n\
        - UP: Higher values\n\
        - DOWN: Lower values\n\
\n\
    [2]: Sequence of channel bits (rgba) available per available pixel for encoding\n\
        - e.g. '1120' means:\n\
            > 1 bit on red channel\n\
            > 1 bit on green channel\n\
            > 2 bits on blue channel\n\
            > 0 bits on alpha channel\n\
\n\
    [3]: Threshold value from 0.00 to 1.00\n\
\n\
Other arguments\n\
---------------\n\
\n\
'-ii': Input image file-path\n\
'-oi': Output image file-path (encoding, exclusively png format)\n\
'-of': Output file-path (decoding)\n\
'-x' : Direct text-data input (encoding, not-recommended)\n\
'-df': Input data file (encoding)\n\
'-rk': Restore thresholds from key-string\n\
'-v' : Verbose mode\n\
'-nomt': Disable multithreading\n\
\n\
Command examples\n\
----------------\n\
\n\
- Encode a text file, using pixels with color saturation higher than 50% (0.5), 1 bit per color channel.\n\
    xsteg -e -t SATURATION UP 1110 0.5 -ii image.jpg -oi image.encoded.png -df text.txt\n\
\n\
- Decode contents of an image with encoded data within:\n\
    xsteg -d -t SATURATION UP 1110 0.5 -ii image.encoded.png -of text.decoded.txt\n\
\n\
- Generate visual data maps for an image:\n\
    xsteg -vd -ii image.jpg\n\
\n\
- Generate 50% luminance diff-map for an image:\n\
    xsteg -m -ii image.jpg -t LUMINANCE UP 0000 0.5 -oi image.luminance.50.png\
";