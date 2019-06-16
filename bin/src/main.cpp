#include <cinttypes>

#include <iostream>
#include <chrono>
#include <cctype>
#include <fstream>

#include <xsteg/steganographer.hpp>
#include <xsteg/runtime_settings.hpp>

#include "utils.hpp"
#include "program_args.hpp"

using namespace std::chrono_literals;
using hclock = std::chrono::high_resolution_clock;
using namespace xsteg;

// xsteg -e -t SATURATION UP 0.5 1210 -ii tt.png -oi tt2.png -x "Hello my friend!"
// xsteg -e -t SATURATION UP 0.5 1210 -ii tt.png -oi tt2.png -df hello.txt
// xsteg -d -t SATURATION UP 0.5 1210 -ii tt2.png -o
// xsteg -d -t SATURATION UP 0.5 1210 -ii tt2.png -of hello2.txt
// xsteg -m -t SATURATION UP 0.5 1210 -ii tt2.png -oi tt3.png
// xsteg -vd -ii tt2.png

std::string generate_key(main_args& args)
{
    image img(1, 1);
    availability_map av_map(&img);
    for(auto& th : args.thresholds)
    {
        av_map.add_threshold(th.data_type, th.direction, th.value, th.bits);
    }
    return av_map.generate_key();
}

void restore_key(main_args& args)
{
    args.thresholds = availability_map::parse_key(args.restore_key);
}

void encode(main_args& args)
{
    steganographer steg(args.input_img);

    if(!args.restore_key.empty()) { restore_key(args); }

    for(auto& th : args.thresholds)
    {
        steg.add_threshold(th.data_type, th.direction, th.value, th.bits);
    }

    steg.write_data(args.data.data(), args.data.size());
    steg.save_to_file(args.output_img);
}

void decode(main_args& args)
{
    steganographer steg(args.input_img);

    if(!args.restore_key.empty()) { restore_key(args); }

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

void diff_map(main_args& args)
{
    image img(args.input_img);
    if(!args.restore_key.empty()) { restore_key(args); }
    image diff_map = generate_visual_data_diff_map(&img, args.thresholds[0].data_type, args.thresholds[0].value);
    diff_map.write_to_file(args.output_img);
}

void vdata_maps(main_args& args)
{
    image img(args.input_img);

    auto log_gen = [](const std::string& type) -> void
    {
        std::cout << "Generating visual data map [" << type << "]" << std::endl;
    };

    log_gen("ALPHA");
    image vmap_alpha = generate_visual_data_map(&img, visual_data_type::ALPHA);
    log_gen("AVERAGE_VALUE_RGB");
    image vmap_avgrgb = generate_visual_data_map(&img, visual_data_type::AVERAGE_VALUE_RGB);
    log_gen("AVERAGE_VALUE_RGBA");
    image vmap_avgrgba = generate_visual_data_map(&img, visual_data_type::AVERAGE_VALUE_RGBA);
    log_gen("COLOR_BLUE");
    image vmap_b = generate_visual_data_map(&img, visual_data_type::COLOR_BLUE);
    log_gen("COLOR_GREEN");
    image vmap_g = generate_visual_data_map(&img, visual_data_type::COLOR_GREEN);
    log_gen("COLOR_RED");
    image vmap_r = generate_visual_data_map(&img, visual_data_type::COLOR_RED);
    log_gen("LUMINANCE");
    image vmap_lum = generate_visual_data_map(&img, visual_data_type::LUMINANCE);
    log_gen("SATURATION");
    image vmap_sat = generate_visual_data_map(&img, visual_data_type::SATURATION);

    std::cout << "Saving files..." << std::endl;
    vmap_alpha.write_to_file(args.input_img + ".ALPHA.png");
    vmap_avgrgb.write_to_file(args.input_img + ".AVERAGE_VALUE_RGB.png");
    vmap_avgrgba.write_to_file(args.input_img + ".AVERAGE_VALUE_RGBA.png");
    vmap_b.write_to_file(args.input_img + ".COLOR_BLUE.png");
    vmap_g.write_to_file(args.input_img + ".COLOR_GREEN.png");
    vmap_r.write_to_file(args.input_img + ".COLOR_RED.png");
    vmap_lum.write_to_file(args.input_img + ".LUMINANCE.png");
    vmap_sat.write_to_file(args.input_img + ".SATURATION.png");
    std::cout << "Done!" << std::endl;
}

int main(int argc, char** argv)
{
    main_args margs = parse_main_args(argc, argv);
    switch (margs.mode)
    {
        case encode_mode::NOT_SET:
        {
            std::cerr << "Encoding mode not set! (-h for help)" << std::endl;
            return -1;
        }
        case encode_mode::ENCODE: { encode(margs); break; }
        case encode_mode::DECODE: { decode(margs); break; }
        case encode_mode::DIFF_MAP: { diff_map(margs); break; }
        case encode_mode::VDATA_MAPS: { vdata_maps(margs); break; }
        case encode_mode::HELP: { std::cout << help_text << std::endl; }
        case encode_mode::GENERATE_KEY:
        { 
            std::cout << std::endl << generate_key(margs) << std::endl;
            break;
        }
    }
    return 0;
}