#pragma once

#include "src/Common/Common.h"
#include "RenderPassBase.h"
#include "src/Function/World/Entity.h"
#include "src/Function/Renderer/Texture.h"
#include "src/Function/Renderer/RasterizerState.h"
#include "src/Function/Renderer/RenderItem.h"

namespace Xunlan
{
    class MeshLightPass final : public RenderPassBase
    {
    public:

        explicit MeshLightPass(uint32 width, uint32 height);

    public:

        void Render(const Ref<RenderContext>& context);

        Ref<RenderTarget> GetMainRT() const { return m_mainRT; }

    private:

        void CollectRenderItems();
        void CollectVisableEntity(const WeakRef<Entity>& refNode);

    private:

        Ref<RenderTarget> m_mainRT;
        Ref<RenderTarget> m_shadowMap;

        std::vector<WeakRef<RenderItem>> m_renderItems;
    };
}