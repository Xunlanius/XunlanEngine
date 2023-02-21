#pragma once

#include "MathTypes.h"

namespace Xunlan::Math
{
    template<typename T>
    constexpr T Clamp(T value, T min, T max)
    {
        return (value < min) ? min : ((value > max) ? max : value);
    }

    template<uint32_t bits>
    constexpr uint32_t PackUnitFloat(float value)
    {
        static_assert(bits <= sizeof(uint32_t) * 8);
        assert(value >= 0 && value <= 1);

        constexpr float intervals = (float)((1u << bits) - 1);
        return (uint32_t)(intervals * value + 0.5f);
    }

    template<uint32_t bits>
    constexpr float UnpackToUnitFloat(uint32_t value)
    {
        static_assert(bits <= sizeof(uint32_t) * 8);
        assert(value < (1u << bits));

        constexpr float intervals = (float)((1u << bits) - 1);
        return (float)value / intervals;
    }

    template<uint32_t bits>
    constexpr uint32_t PackFloat(float value, float minRange, float maxRange)
    {
        assert(value >= minRange && value <= maxRange);

        float scaledValue = (value - minRange) / (maxRange - minRange);
        return PackUnitFloat<bits>(scaledValue);
    }

    template<uint32_t bits>
    constexpr float UnpackToUnitFloat(uint32_t value, float minRange, float maxRange)
    {
        assert(value >= minRange && value <= maxRange);
        return UnpackToUnitFloat<bits>(value) * (maxRange - minRange) + minRange;
    }
}