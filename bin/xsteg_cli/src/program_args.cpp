#include "program_args.hpp"

#include <fstream>
#include <iostream>

#include <xsteg/runtime_settings.hpp>
#include <strutils/strutils.hpp>

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
    auto next_arg = [&]() -> std::string 
	{ 
		if (current_arg >= argc)
		{
			throw std::out_of_range("Not enough arguments!");
		}
		return std::string(argv[current_arg++]); 
	};

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
        else if(arg == "-oif")
        {
            std::string fmtstr = next_arg();
            strutils::to_upper_in_place(fmtstr);

            if(fmtstr == "JPG" || fmtstr == "JPEG")
            {
                result.output_img_format = image_format::jpeg;
            }
            else if(fmtstr == "PNG")
            {
                result.output_img_format = image_format::png;
            }
            else
            {
                std::cout << "Invalid image format: '" << fmtstr << "', aborting...";
                exit(-1);
            }
        }
        else if(arg == "-oiq")
        {
            result.output_img_jpeg_quality = std::stoi(next_arg());
        }
        else if(arg == "-ra")
        { 
            result.mode = encode_mode::RESIZE_ABSOLUTE;
            result.resize_w = std::stof(next_arg());
            result.resize_h = std::stof(next_arg());
        }
        else if(arg == "-rp")
        { 
            result.mode = encode_mode::RESIZE_PROPORTIONAL; 
            result.resize_w = std::stof(next_arg());
            result.resize_h = std::stof(next_arg());
        }
        else if(arg == "-t")
        {
            std::string type = next_arg();
            std::string dir  = next_arg();
            std::string bits = next_arg();
            std::string val  = next_arg();
            strutils::to_upper_in_place(type);
            strutils::to_upper_in_place(dir);

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
			if (!ifs)
			{
				throw std::invalid_argument("Unable to open datafile: " + datafile);
			}
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
Resize mode arguments\n\
----------------------\n\
    '-ra *w *h': Generate resized image (absolute pixel dimensions)\n\
    '-rp *w *h': Generate resized image (proportional percentages)\n\
    *w = Resized image width, in pixels or percentage (-ra, -rp, respectively)\n\
    *w = Resized image height, in pixels or percentage (-ra, -rp, respectively)\n\
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
'-oif': Output image format (either PNG or JPEG)\n\
'-oiq': Output image quality (1-100, exclusive to JPEG format)\n\
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
    xsteg -m -ii image.jpg -t LUMINANCE UP 0000 0.5 -oi image.luminance.50.png\n\
\n\
- Generate a encoding/deconding key from a list of thresholds:\n\
xsteg.exe -gk\n\
    -t SATURATION UP 1110 0.344\n\
    -t COLOR_RED UP 1000 0.5\n\
    -t COLOR_GREEN UP 0100 0.5\n\
    -t COLOR_BLUE UP 0010 0.5\n\
---------------------------------------------------------------\n\
output: &S>A*1110+0.344&0>A*1000+0.5&1>A*0100+0.5&2>A*0010+0.5\n\
---------------------------------------------------------------\n\
\n\
- Generate a encoding/deconding key from a list of thresholds and save it to a file:\n\
xsteg.exe -gk \n\
    -t SATURATION UP 1110 0.344\n\
    -t COLOR_RED UP 1000 0.5\n\
    -t COLOR_GREEN UP 0100 0.5\n\
    -t COLOR_BLUE UP 0010 0.5\n\
    -of 'encodign_key.txt'\n\
\n\
- Encode a data file into an image, using the previously generated encoding key:\n\
xsteg -e -ii image.jpg -oi image_encoded.png -df data.txt\n\
    -rk '&S>A*1110+0.344&0>A*1000+0.5&1>A*0100+0.5&2>A*0010+0.5'\n\
\n\
- Generate resized image, in absolute pixel dimensions:\n\
xsteg -ra 800 600 -ii image.jpg -oi image_resized.png\n\
\n\
- Generate resized image, in proportional percentage dimensions:\n\
xsteg -rp 65.5 35.25 -ii image.jpg -oi image_resized.png\
";