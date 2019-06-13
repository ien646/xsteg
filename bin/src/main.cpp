#include <cinttypes>

#include <iostream>
#include <chrono>
#include <cctype>
#include <fstream>
#include <xsteg/steganographer.hpp>

using namespace std::chrono_literals;
using hclock = std::chrono::high_resolution_clock;
using namespace xsteg;

// xsteg -e -t SATURATION UP 0.5 1210 -ii tt.png -oi tt2.png -x "Hello my friend!"
// xsteg -e -t SATURATION UP 0.5 1210 -ii tt.png -oi tt2.png -df hello.txt
// xsteg -d -t SATURATION UP 0.5 1210 -ii tt2.png -o
// xsteg -d -t SATURATION UP 0.5 1210 -ii tt2.png -of hello2.txt

std::map<std::string, visual_data_type> visual_data_type_name_map = 
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

std::map<std::string, threshold_direction> threshold_direction_name_map =
{
    { "UP", threshold_direction::UP },
    { "DOWN", threshold_direction::DOWN}
};

enum class encode_mode
{
    NOT_SET,
    ENCODE,
    DECODE,
};

void str_toupper(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char ch)
    {
        return std::toupper(ch);
    });
}

pixel_availability parse_pxav_bits(const std::string& bits_str)
{
    pixel_availability result;
    int ibits = std::stoi(bits_str);
    result.r = ibits / 1000;
    result.g = (ibits % 1000) / 100;
    result.b = (ibits % 100) / 10;
    result.a = (ibits % 10);
    return result;
}

struct main_args
{
    encode_mode mode = encode_mode::NOT_SET;
    std::string input_img;
    std::string output_img;
    std::vector<availability_threshold> thresholds;
    std::vector<uint8_t> data;
    bool output_std = false;
    std::string output_file;
};

std::vector<uint8_t> str_to_datavec(const std::string& str)
{
    std::vector<uint8_t> result;
    result.resize(str.size(), 0x00u);
    std::transform(str.begin(), str.end(), result.begin(), [](char ch)
    {
        return static_cast<uint8_t>(ch);
    });
    return result;
}

main_args parse_main_args(int argc, char** argv)
{
    size_t current_arg = 1;

    main_args result;

    auto next_arg = [&]() -> std::string { return std::string(argv[current_arg++]); };

    while (current_arg < argc)
    {
        std::string arg(next_arg());
        if(arg == "-ii")
        {
            result.input_img = next_arg();
        }
        else if(arg == "-oi")
        {
            result.output_img = next_arg();
        }
        else if(arg == "-t")
        {
            std::string type = next_arg();
            std::string dir  = next_arg();
            std::string val  = next_arg();
            std::string bits = next_arg();
            str_toupper(type);
            str_toupper(dir);

            availability_threshold threshold;
            threshold.data_type = visual_data_type_name_map[type];
            threshold.direction = threshold_direction_name_map[dir];
            threshold.value = std::stof(val);
            threshold.bits = parse_pxav_bits(bits);
            result.thresholds.push_back(threshold);
        }
        else if(arg == "-e")
        {
            result.mode = encode_mode::ENCODE;
        }
        else if(arg == "-d")
        {
            result.mode = encode_mode::DECODE;
        }
        else if(arg == "-o")
        {
            result.output_std = true;
        }
        else if(arg == "-of")
        {
            result.output_file = next_arg();
        }
        else if(arg == "-x")
        {
            result.data = str_to_datavec(next_arg());
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

void encode(main_args args)
{
    steganographer steg(args.input_img);
    for(auto& th : args.thresholds)
    {
        steg.add_threshold(th.data_type, th.direction, th.value, th.bits);
    }

    steg.write_data(args.data.data(), args.data.size());
    steg.save_to_file(args.output_img);
}

void decode(main_args args)
{
    steganographer steg(args.input_img);
    for(auto& th : args.thresholds)
    {
        steg.add_threshold(th.data_type, th.direction, th.value, th.bits);
    }

    auto data = steg.read_data();
    if(args.output_std)
    {
        std::string str(reinterpret_cast<char*>(data.data()), data.size());
        std::cout << str << std::endl;
    }
    if(!args.output_file.empty())
    {
        std::ofstream ofs(args.output_file, std::ios_base::binary);
        ofs.write(reinterpret_cast<char*>(data.data()), data.size());
        ofs.close();
    }
}

int main(int argc, char** argv)
{
    main_args margs = parse_main_args(argc, argv);
    if(margs.mode == encode_mode::NOT_SET)
    {
        std::cerr << "Encoding mode not set!" << std::endl;
    }
    else if(margs.mode == encode_mode::ENCODE)
    {
        encode(margs);
    }
    else if(margs.mode == encode_mode::DECODE)
    {
        decode(margs);
    }
    return 0;
}