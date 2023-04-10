#pragma once

#include "src/Common/Common.h"
#include "src/Function/World/ECS/ECS.h"
#include "src/Function/Renderer/RenderItem.h"

namespace Xunlan
{
    struct MeshRenderComponent final
    {
        Ref<RenderItem> m_renderItem;
        bool m_castShadow = true;
    };

    class MeshRenderSystem : public ECS::ISystem
    {
        void Update();
    };
}