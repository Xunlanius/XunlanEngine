#pragma once

#include "Vector.h"
#include "Matrix.h"

namespace Xunlan::Math
{
    template<typename T>
    inline T Clamp(T value, T min, T max) { return (value < min) ? min : ((value > max) ? max : value); }

    template<uint64 Align>
    inline uint64 RoundUp(uint64 size)
    {
        static_assert(Align > 0, "Align must be non-zero.");
        static_assert((Align & (Align - 1)) == 0, "Align must be a power of 2.");
        return (size + Align - 1) & ~(Align - 1);
    }

    template<uint64 Align>
    inline uint64 RoundDown(uint64 size)
    {
        static_assert(Align > 0, "Align must be non-zero.");
        static_assert((Align & (Align - 1)) == 0, "Align must be a power of 2.");
        return size & ~(Align - 1);
    }

    inline uint64 RoundUp(uint64 size, uint64 align)
    {
        assert(align > 0 && "Align must be non-zero.");
        assert((align & (align - 1)) == 0 && "Align must be a power of 2.");
        return (size + align - 1) & ~(align - 1);
    }

    inline uint64 RoundDown(uint64 size, uint64 align)
    {
        assert(align > 0 && "Align must be non-zero.");
        assert((align & (align - 1)) == 0 && "Align must be a power of 2.");
        return size & ~(align - 1);
    }

    template<uint32 numBits>
    inline uint32 PackUnitFloat(float value)
    {
        static_assert(numBits <= sizeof(uint32) * 8);
        assert(value >= 0 && value <= 1);

        constexpr float intervals = (float)((1u << numBits) - 1);
        return (uint32)(intervals * value + 0.5f);
    }

    template<uint32 numBits>
    inline float UnpackToUnitFloat(uint32 value)
    {
        static_assert(numBits <= sizeof(uint32) * 8);
        assert(value < (1u << numBits));

        constexpr float intervals = (float)((1u << numBits) - 1);
        return (float)value / intervals;
    }

    template<uint32 numBits>
    inline uint32 PackFloat(float value, float minRange, float maxRange)
    {
        assert(value >= minRange && value <= maxRange);

        float scaledValue = (value - minRange) / (maxRange - minRange);
        return PackUnitFloat<numBits>(scaledValue);
    }

    template<uint32 numBits>
    inline float UnpackToUnitFloat(uint32 value, float minRange, float maxRange)
    {
        assert(value >= minRange && value <= maxRange);
        return UnpackToUnitFloat<numBits>(value) * (maxRange - minRange) + minRange;
    }

    inline uint64 ComputeCRC32Uint64(const byte* data, uint64 size)
    {
        assert(size >= sizeof(uint64));

        uint64 crc = 0;
        const byte* at = data;
        const byte* end = data + RoundDown<sizeof(uint64)>(size);

        while (at < end)
        {
            crc = _mm_crc32_u64(crc, *(uint64*)at);
            at += sizeof(uint64);
        }

        return crc;
    }
}