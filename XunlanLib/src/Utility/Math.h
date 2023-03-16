#pragma once

#include "MathTypes.h"

namespace Xunlan::Math
{
    template<typename T>
    constexpr T Clamp(T value, T min, T max) { return (value < min) ? min : ((value > max) ? max : value); }

    template<uint64 Align>
    constexpr uint64 RoundUp(uint64 size)
    {
        static_assert(Align > 0, "Align must be non-zero.");
        static_assert(Align & (Align - 1) == 0, "Align must be a power of 2.");
        return (size + Align - 1) & ~(Align - 1);
    }

    template<uint32 numBits>
    constexpr uint32 PackUnitFloat(float value)
    {
        static_assert(numBits <= sizeof(uint32) * 8);
        assert(value >= 0 && value <= 1);

        constexpr float intervals = (float)((1u << numBits) - 1);
        return (uint32)(intervals * value + 0.5f);
    }

    template<uint32 numBits>
    constexpr float UnpackToUnitFloat(uint32 value)
    {
        static_assert(numBits <= sizeof(uint32) * 8);
        assert(value < (1u << numBits));

        constexpr float intervals = (float)((1u << numBits) - 1);
        return (float)value / intervals;
    }

    template<uint32 numBits>
    constexpr uint32 PackFloat(float value, float minRange, float maxRange)
    {
        assert(value >= minRange && value <= maxRange);

        float scaledValue = (value - minRange) / (maxRange - minRange);
        return PackUnitFloat<numBits>(scaledValue);
    }

    template<uint32 numBits>
    constexpr float UnpackToUnitFloat(uint32 value, float minRange, float maxRange)
    {
        assert(value >= minRange && value <= maxRange);
        return UnpackToUnitFloat<numBits>(value) * (maxRange - minRange) + minRange;
    }
}