#pragma once

#include "Common/Common.h"
#include "Utilities/Math.h"

namespace Xunlan::Transformer
{
    struct Transformer
    {
        Math::Vector3 position;
        Math::Vector4 rotation;
        Math::Vector3 scale;
    };
}