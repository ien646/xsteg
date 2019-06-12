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
        visual_data_type::SATURATION,
        threshold_direction::UP,
        0.5F,
        pixel_availability(1, 2, 1, 0)
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

    int r = 0;
    std::cin >> r;

    return 0;
}