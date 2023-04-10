#pragma once

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

    static constexpr EntityID INVALID_ID = UINT32_MAX;

    static constexpr size_t MAX_COMPONENT_COUNT = 64;
    using Signature = std::bitset<MAX_COMPONENT_COUNT>;
}