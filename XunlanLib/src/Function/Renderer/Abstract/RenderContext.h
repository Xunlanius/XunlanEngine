#pragma once

#include "src/Common/Common.h"

namespace Xunlan
{
    class RenderContext
    {
    protected:

        RenderContext() = default;
        DISABLE_COPY(RenderContext)
        DISABLE_MOVE(RenderContext)
        virtual ~RenderContext() = default;
    };
}