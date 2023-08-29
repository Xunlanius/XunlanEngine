#pragma once

#include "src/Common/Common.h"
#include "RenderPassBase.h"
#include "src/Function/World/Entity.h"
#include "src/Function/Renderer/Abstract/Texture.h"
#include "src/Function/Renderer/Abstract/Material.h"
#include "src/Function/Renderer/Abstract/RenderItem.h"

namespace Xunlan
{
    class ShadowPass final : public RenderPassBase
    {
    public:

        ShadowPass(uint32 width, uint32 height);

    public:

        void Render(Ref<RenderContext> context);

        Ref<DepthBuffer> GetShadowMap() const { return m_depth; }

    private:

        void CollectRenderItems();
        void CollectVisableEntity(const WeakRef<Entity>& refNode);
        void RenderItems(Ref<RenderContext> context);

    private:

        uint32 m_width;
        uint32 m_height;

        Ref<RenderTarget> m_fluxLS;
        Ref<RenderTarget> m_positionLS;
        Ref<RenderTarget> m_normalLS;
        Ref<DepthBuffer> m_depth;

        Ref<CBuffer> m_shadowMaps;
        Ref<Material> m_shadowMaterial;

        std::vector<WeakRef<RenderItem>> m_renderItems;
    };
}