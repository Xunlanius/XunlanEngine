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

        ShadowPass();

    public:

        void Render(const Ref<RenderContext>& context);

        Ref<RenderTarget> GetShadowMap() const { return m_shadowMap; }

    private:

        void CollectRenderItems();
        void CollectVisableEntity(const WeakRef<Entity>& refNode);

    private:

        static constexpr uint32 SHADOW_MAP_WIDTH = 1024;
        static constexpr uint32 SHADOW_MAP_HEIGHT = 1024;

        Ref<RenderTarget> m_shadowMap;
        Ref<CBuffer> m_shadowMapIndices;
        Ref<Material> m_shadowMaterial;

        std::vector<WeakRef<RenderItem>> m_renderItems;
    };
}