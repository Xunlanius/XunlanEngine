#pragma once

#include "src/Common/Common.h"
#include "RenderPassBase.h"
#include "src/Function/World/Entity.h"
#include "src/Function/Renderer/Abstract/Texture.h"
#include "src/Function/Renderer/Abstract/RasterizerState.h"
#include "src/Function/Renderer/Abstract/RenderItem.h"

namespace Xunlan
{
    class GPass final : public RenderPassBase
    {
    public:

        explicit GPass(uint32 width, uint32 height);

    public:

        void Render(Ref<RenderContext> context);

    private:

        void CollectRenderItems();
        void CollectVisableEntity(const WeakRef<Entity>& refNode);
        void RenderItems(Ref<RenderContext> context);

    private:

        uint32 m_width = 0;
        uint32 m_height = 0;

        Ref<RenderTarget> m_albedo;
        Ref<RenderTarget> m_position;
        Ref<RenderTarget> m_normal;
        Ref<DepthBuffer> m_depthBuffer;

        Ref<CBuffer> m_gBuffer;

        std::vector<WeakRef<RenderItem>> m_renderItems;
    };
}