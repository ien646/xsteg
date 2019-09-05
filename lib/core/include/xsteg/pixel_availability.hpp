#pragma once

namespace xsteg
{
    struct pixel_availability
    {
        int r = -1;
        int g = -1;
        int b = -1;
        int a = -1;

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

        constexpr bool is_useless() const
        {
            return r == -1 && g == -1 && b == -1 && a == -1;
        }
    };
}