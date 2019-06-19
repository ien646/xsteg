#include <xsteg/synced_print.hpp>

#include <iostream>
#include <mutex>

namespace xsteg
{
    void synced_print(const std::string& str, bool endl)
    {
        static std::mutex mux;
        std::lock_guard lock(mux);
        std::cout << str;
        if(endl)
        {
            std::cout << std::endl;
        }
    }
}