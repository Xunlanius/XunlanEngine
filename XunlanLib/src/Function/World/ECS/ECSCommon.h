#pragma once

#include "src/Common/Common.h"
#include <cstdint>
#include <cassert>
#include <bitset>
#include <functional>
#include <type_traits>

#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <tuple>

namespace Xunlan::ECS
{
    using EntityID = uint32_t;
    using ComponentID = uint8_t;
    using SystemID = uint32_t;

    constexpr EntityID INVALID_ID = UINT32_MAX;

    constexpr size_t MAX_COMPONENT_COUNT = 64;
    using Signature = std::bitset<MAX_COMPONENT_COUNT>;

    class Component;
    class System;

    template<typename T>
    struct IDTraits;
    template<>
    struct IDTraits<Component> { using type = ComponentID; };
    template<>
    struct IDTraits<System> { using type = SystemID; };

    template<typename Category, typename id_type = IDTraits<Category>::type>
    class IDGetter final
    {
    public:

        template<typename T>
        static id_type GetID() { static const id_type id = ms_currID++; return id; }
        template<typename T>
        static bool IsRegistered()
        {
            const id_type currID = ms_currID;
            return GetID<T>() < currID;
        }

    private:

        static inline id_type ms_currID = 0;
    };
}