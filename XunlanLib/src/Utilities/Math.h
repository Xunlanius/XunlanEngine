#pragma once

#include "MathTypes.h"

namespace Xunlan::Math
{
    template<typename T>
    constexpr T Clamp(T value, T min, T max)
    {
        return (value < min) ? min : ((value > max) ? max : value);
    }

    template<uint32 bits>
    constexpr uint32 PackUnitFloat(float value)
    {
        static_assert(bits <= sizeof(uint32) * 8);
        assert(value >= 0 && value <= 1);

        constexpr float intervals = (float)((1u << bits) - 1);
        return (uint32)(intervals * value + 0.5f);
    }

    template<uint32 bits>
    constexpr float UnpackToUnitFloat(uint32 value)
    {
        static_assert(bits <= sizeof(uint32) * 8);
        assert(value < (1u << bits));

        constexpr float intervals = (float)((1u << bits) - 1);
        return (float)value / intervals;
    }

    template<uint32 bits>
    constexpr uint32 PackFloat(float value, float minRange, float maxRange)
    {
        assert(value >= minRange && value <= maxRange);

        float scaledValue = (value - minRange) / (maxRange - minRange);
        return PackUnitFloat<bits>(scaledValue);
    }

    template<uint32 bits>
    constexpr float UnpackToUnitFloat(uint32 value, float minRange, float maxRange)
    {
        assert(value >= minRange && value <= maxRange);
        return UnpackToUnitFloat<bits>(value) * (maxRange - minRange) + minRange;
    }
}