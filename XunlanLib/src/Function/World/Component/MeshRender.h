#pragma once

#include "src/Common/Common.h"
#include "src/Function/World/ECS/World.h"
#include "src/Function/Renderer/Abstract/RenderItem.h"

namespace Xunlan
{
    struct MeshRenderComponent final
    {
        Ref<RenderItem> m_renderItem;
        bool m_castShadow = true;
    };

    class MeshRenderSystem
    {
    public:

        static void Update();
    };
}