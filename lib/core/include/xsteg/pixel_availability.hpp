#pragma once

namespace xsteg
{
    struct pixel_availability
    {
        int r = 0;
        int g = 0;
        int b = 0;
        int a = 0;

        constexpr pixel_availability() noexcept = default;
        constexpr pixel_availability(int rv, int gv, int bv, int av) noexcept
        {
            r = rv;
            g = gv;
            b = bv;
            a = av;
        }

        constexpr bool is_zero() const
        {
            return r == 0 && g == 0 && b == 0 && a == 0;
        }
    };
}