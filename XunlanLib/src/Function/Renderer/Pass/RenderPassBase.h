#pragma once

#include "src/Common/Common.h"
#include "src/Function/Renderer/Abstract/CBuffer.h"
#include "src/Function/Renderer/Abstract/RenderItem.h"
#include "src/Function/World/Entity.h"
#include "src/Function/World/Component/Transformer.h"
#include "src/Utility/Math/Math.h"

namespace Xunlan
{
    class RenderPassBase
    {
    public:

        virtual void ResizeResolution() {}

    protected:

        static Ref<RenderItem> CreateCanvas();
    };
}