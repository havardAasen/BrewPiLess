/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 Håvard F. Aasen
 */

#ifndef BPL_COMMON_CONVERSION_H
#define BPL_COMMON_CONVERSION_H

#include <type_traits>

namespace bpl {
    template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
    [[nodiscard]] constexpr T celsius_to_fahrenheit(const T celsius)
    {
        return celsius * T{1.8} + T{32.0};
    }

    template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
    [[nodiscard]] constexpr T fahrenheit_to_celsius(const T fahrenheit)
    {
        return (fahrenheit - T{32.0}) / T{1.8};
    }

    template<typename T,typename = std::enable_if<std::is_floating_point_v<T>>>
    [[nodiscard]] constexpr T brix_to_specific_gravity(const T brix)
    {
        return 1 + T{0.004} * brix;
    }

    template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
    [[nodiscard]] constexpr T specific_gravity_to_brix(const T specific_gravity)
    {
        if (specific_gravity <= 1.0)
            return T{0.0};

        return ((T{182.4601} * specific_gravity - T{775.6821}) * specific_gravity + T{1262.7794}) *
               specific_gravity - T{669.5622};
    }
}

#endif
