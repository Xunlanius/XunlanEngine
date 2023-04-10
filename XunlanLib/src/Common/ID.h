#pragma once

#include "src/Common/DataTypes.h"

namespace Xunlan
{
    using ID = uint32;
    using LongID = uint64;

    constexpr ID INVALID_ID = (ID)-1;
    constexpr LongID INVALID_LONG_ID = (LongID)-1;

    constexpr bool IsValid(ID id) { return id != INVALID_ID; }
    constexpr bool IsValid(LongID id) { return id != INVALID_LONG_ID; }
}