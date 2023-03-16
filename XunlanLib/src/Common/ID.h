#pragma once

#include "Common/DataTypes.h"

namespace Xunlan
{
    using ID = uint32;

    constexpr ID INVALID_ID = (ID)-1;

    constexpr bool IsValid(ID id) { return id != INVALID_ID; }
}