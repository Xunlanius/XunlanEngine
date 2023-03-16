#pragma once

#include "Common/Common.h"

namespace Xunlan::Content
{
    enum class PrimitiveTopology : uint32
    {
        POINT_LIST = 1,
        LINE_LIST,
        LINE_STRIP,
        TRIANGLE_LIST,
        TRIANGLE_STRIP,

        COUNT,
    };
}