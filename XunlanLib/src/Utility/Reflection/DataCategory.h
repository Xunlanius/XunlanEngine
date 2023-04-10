#pragma once

#include <cstdint>
#include <type_traits>
#include <array>
#include <vector>
#include <string>

namespace Xunlan::Reflection
{
    using byte = uint8_t;
    using int16 = int16_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using int64 = int64_t;
    using uint64 = uint64_t;

    enum class DataCategory : byte
    {
        BOOL,
        CHAR,
        BYTE,
        INT16,
        UINT16,
        INT32,
        UINT32,
        INT64,
        UINT64,
        FLOAT,
        DOUBLE,

        NATIVE_ARRAY,
        ARRAY,
        VECTOR,
        STRING,

        CUSTOM,
    };

    template<typename T>
    struct data_category
    {
        static_assert(std::is_class_v<T>, "Unknown category.");
        static constexpr DataCategory CATEGORY = DataCategory::CUSTOM;
    };

    template<>
    struct data_category<bool> { static constexpr DataCategory CATEGORY = DataCategory::BOOL; };
    template<>
    struct data_category<char> { static constexpr DataCategory CATEGORY = DataCategory::CHAR; };
    template<>
    struct data_category<byte> { static constexpr DataCategory CATEGORY = DataCategory::BYTE; };
    template<>
    struct data_category<int16> { static constexpr DataCategory CATEGORY = DataCategory::INT16; };
    template<>
    struct data_category<uint16> { static constexpr DataCategory CATEGORY = DataCategory::UINT16; };
    template<>
    struct data_category<int> { static constexpr DataCategory CATEGORY = DataCategory::INT32; };
    template<>
    struct data_category<uint32> { static constexpr DataCategory CATEGORY = DataCategory::UINT32; };
    template<>
    struct data_category<int64> { static constexpr DataCategory CATEGORY = DataCategory::INT64; };
    template<>
    struct data_category<uint64> { static constexpr DataCategory CATEGORY = DataCategory::UINT64; };
    template<>
    struct data_category<float> { static constexpr DataCategory CATEGORY = DataCategory::FLOAT; };
    template<>
    struct data_category<double> { static constexpr DataCategory CATEGORY = DataCategory::DOUBLE; };

    template<typename T, size_t N>
    struct data_category<T[N]> { static constexpr DataCategory CATEGORY = DataCategory::NATIVE_ARRAY; };
    template<typename T, size_t N>
    struct data_category<std::array<T, N>> { static constexpr DataCategory CATEGORY = DataCategory::ARRAY; };
    template<typename T>
    struct data_category<std::vector<T>> { static constexpr DataCategory CATEGORY = DataCategory::VECTOR; };
    template<>
    struct data_category<std::string> { static constexpr DataCategory CATEGORY = DataCategory::STRING; };

    template<typename T>
    constexpr DataCategory DATA_CATEGORY = data_category<std::remove_cvref_t<T>>::CATEGORY;

    template<typename>
    struct RemoveMemPtr {};

    template<typename T, typename Obj>
    struct RemoveMemPtr<T Obj::*> { using type = T; };

    template<typename T>
    using remove_memPtr_t = RemoveMemPtr<T>::type;
}