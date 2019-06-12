#include <cinttypes>

#include <iostream>
#include <chrono>
#include <xsteg/steganographer.hpp>

using namespace std::chrono_literals;
using hclock = std::chrono::high_resolution_clock;
using namespace xsteg;

int main(int argc, char** argv)
{
    steganographer steg("tt.png");
    steg.add_threshold(
        visual_data_type::COLOR_RED,
        threshold_direction::DOWN,
        0.2F,
        pixel_availability(2, 1, 1, 0)
    );
    
    std::string text = "Hello my friend!";
    std::vector<uint8_t> data;
    data.resize(text.size(), 0x00u);
    std::transform(text.begin(), text.end(), data.begin(), [](char ch) -> uint8_t
    {
        return static_cast<uint8_t>(ch);
    });

    std::string key = steg.get_key();
    steg.write_data(data.data(), data.size());

    steg.save_to_file("tt2.png");

    steganographer steg2("tt2.png");

    steg2.restore_key(key);
    auto re_data = steg2.read_data();

    std::string result_text;
    result_text.resize(re_data.size(), '\0');

    std::transform(re_data.begin(), re_data.end(), result_text.begin(), [](uint8_t ch) -> char
    {
        return static_cast<char>(ch);
    });

    image tt("tt.png");
    auto ttdiff = generate_visual_data_diff_map(&tt, visual_data_type::COLOR_RED, 0.2F);
    ttdiff.write_to_file("tt_diff.png");

    std::cout << "[" << result_text << "]" << std::endl;

    int r = 0;
    std::cin >> r;

    return 0;
}