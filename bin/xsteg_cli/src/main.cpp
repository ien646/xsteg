#include <cinttypes>

#include <iostream>
#include <chrono>
#include <cctype>
#include <fstream>
#include <mutex>

#include <xsteg/steganographer.hpp>
#include <xsteg/task_queue.hpp>

#include "utils.hpp"
#include "program_args.hpp"

using namespace std::chrono_literals;
using hclock = std::chrono::high_resolution_clock;
using namespace xsteg;

void require_thresholds(main_args& args)
{
    if(args.thresholds.empty())
    {
        std::cout << "No threshold(s) specified, aborting..." << std::endl;
        exit(-1);
    }
}

void require_thresholds_or_rkey(main_args& args)
{
    if(args.thresholds.empty() && args.restore_key.empty())
    {
        std::cout << "No threshold(s) specified, aborting..." << std::endl;
        exit(-1);
    }
}

void require_rkey(main_args& args)
{
    if(args.restore_key.empty())
    {
        std::cout << "No key specified, aborting..." << std::endl;
        exit(-1);
    }
}

void require_input_image(main_args& args)
{
    if(args.input_img.empty())
    {
        std::cout << "No input image specified, aborting..." << std::endl;
        exit(-1);
    }
}

void require_output_image(main_args& args)
{
    if(args.output_img.empty())
    {
        std::cout << "No output image specified, aborting..." << std::endl;
        exit(-1);
    }
}

void require_output_file(main_args& args)
{
    if(args.output_img.empty())
    {
        std::cout << "No output file specified, aborting..." << std::endl;
        exit(-1);
    }
}

void require_data(main_args& args)
{
    if(args.data.empty())
    {
        std::cout << "No data to encode found, aborting..." << std::endl;
        exit(-1);
    }
}

void require_resize_values(main_args& args)
{
    if(args.resize_w <= 0 || args.resize_h <= 0)
    {
        std::cout << "Invalid resize dimensions, aborting..." << std::endl;
        exit(-1);
    }
}

std::string generate_key(main_args& args)
{
    require_thresholds(args);

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
    require_rkey(args);
    args.thresholds = availability_map::parse_key(args.restore_key);
}

void encode(main_args& args)
{
    require_thresholds_or_rkey(args);
    require_input_image(args);
    require_output_image(args);
    require_data(args);

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
    require_thresholds_or_rkey(args);
    require_input_image(args);
    if(!args.output_std)
    {
        require_output_file(args);
    }

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
    require_thresholds_or_rkey(args);
    require_input_image(args);
    require_output_image(args);

    image img(args.input_img);
    if(!args.restore_key.empty()) { restore_key(args); }

    image diff_map = generate_visual_data_diff_image(&img, args.thresholds[0].data_type, args.thresholds[0].value);
    
    image_save_options opt;
    opt.format = args.output_img_format;
    opt.jpeg_quality = args.output_img_jpeg_quality;

    diff_map.write_to_file(args.output_img, opt);
}

void vdata_maps(main_args& args)
{
    require_input_image(args);

    image img(args.input_img);

    std::mutex log_mux;
    auto log_gen = [&](const std::string& type) -> void
    {
        std::lock_guard<std::mutex> lock(log_mux);
        std::cout << "Generating visual data image [" << type << "]" << std::endl;
    };

    image_save_options opt;
    opt.format = args.output_img_format;
    opt.jpeg_quality = args.output_img_jpeg_quality;

    std::string file_ext = (opt.format == image_format::png) ? ".png" : ".jpg";

    task_queue tq;

    tq.enqueue([&]()
    {
        log_gen("ALPHA");
        image vmap_alpha = generate_visual_data_image(&img, visual_data_type::ALPHA);
        vmap_alpha.write_to_file(args.input_img + ".ALPHA" + file_ext, opt);
    });
    
    tq.enqueue([&]()
    {
        log_gen("AVERAGE_VALUE_RGB");
        image vmap_avgrgb = generate_visual_data_image(&img, visual_data_type::AVERAGE_VALUE_RGB);
        vmap_avgrgb.write_to_file(args.input_img + ".AVERAGE_VALUE_RGB" + file_ext, opt);
    });

    tq.enqueue([&]()
    {
        log_gen("AVERAGE_VALUE_RGBA");
        image vmap_avgrgba = generate_visual_data_image(&img, visual_data_type::AVERAGE_VALUE_RGBA);
        vmap_avgrgba.write_to_file(args.input_img + ".AVERAGE_VALUE_RGBA" + file_ext, opt);
    });

    tq.enqueue([&]()
    {
        log_gen("COLOR_BLUE");
        image vmap_b = generate_visual_data_image(&img, visual_data_type::COLOR_BLUE);
        vmap_b.write_to_file(args.input_img + ".COLOR_BLUE" + file_ext, opt);
    });  

    tq.enqueue([&]()
    {
        log_gen("COLOR_GREEN");
        image vmap_g = generate_visual_data_image(&img, visual_data_type::COLOR_GREEN);
        vmap_g.write_to_file(args.input_img + ".COLOR_GREEN" + file_ext, opt);
    });  

    tq.enqueue([&]()
    {
        log_gen("COLOR_RED");
        image vmap_r = generate_visual_data_image(&img, visual_data_type::COLOR_RED);
        vmap_r.write_to_file(args.input_img + ".COLOR_RED" + file_ext, opt);
    });  

    tq.enqueue([&]()
    {
        log_gen("LUMINANCE");
        image vmap_lum = generate_visual_data_image(&img, visual_data_type::LUMINANCE);
        vmap_lum.write_to_file(args.input_img + ".LUMINANCE" + file_ext, opt);
    });

    tq.enqueue([&]()
    {
        log_gen("SATURATION");
        image vmap_sat = generate_visual_data_image(&img, visual_data_type::SATURATION);
        vmap_sat.write_to_file(args.input_img + ".SATURATION" + file_ext, opt);
    });

    tq.run(false);
    
    std::cout << "Done!" << std::endl;
}

void gen_key(main_args args)
{
    require_thresholds(args);

    std::string key = generate_key(args);
    if(!args.output_file.empty())
    {
        std::ofstream ofs(args.output_file);
        if(ofs)
        {
            ofs << key;
            ofs.close();
        }
    }
    else
    {
        std::cout << std::endl << key << std::endl;
    }
}

void resize_abs(main_args args)
{
    require_input_image(args);
    require_output_image(args);
    require_resize_values(args);

    image img(args.input_img);
    image resized = img.create_resized_copy_absolute(
        static_cast<int>(args.resize_w),
        static_cast<int>(args.resize_h)
    );
    resized.write_to_file(args.output_img);
}

void resize_pro(main_args args)
{
    require_input_image(args);
    require_output_image(args);
    require_resize_values(args);

    image img(args.input_img);
    image resized = img.create_resized_copy_proportional(args.resize_w, args.resize_h);
    resized.write_to_file(args.output_img);
}

#include <filesystem>

int main(int argc, char** argv)
{	
	std::cout << std::filesystem::current_path() << std::endl;
	try
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
		case encode_mode::HELP: { std::cout << help_text << std::endl; break; }
		case encode_mode::GENERATE_KEY: { gen_key(margs); break; }
		case encode_mode::RESIZE_ABSOLUTE: { resize_abs(margs); break; }
		case encode_mode::RESIZE_PROPORTIONAL: { resize_pro(margs); break; }
		}
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}
    return 0;
}