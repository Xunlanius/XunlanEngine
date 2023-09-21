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

        void Render(Ref<RenderContext> context, const std::vector<Ref<RenderItem>>& items);

        CRef<RenderTarget> GetFlux() const { return m_flux; }
        CRef<RenderTarget> GetPosWS() const { return m_posWS; }
        CRef<RenderTarget> GetNormalWS() const { return m_normalWS; }
        CRef<DepthBuffer> GetDepthMap() const { return m_depth; }

    private:

        void CreateMaterial();
        void RenderItems(Ref<RenderContext> context, const std::vector<Ref<RenderItem>>& items);

    private:

        uint32 m_width;
        uint32 m_height;

        Ref<RenderTarget> m_flux;
        Ref<RenderTarget> m_posWS;
        Ref<RenderTarget> m_normalWS;
        Ref<DepthBuffer> m_depth;

        Ref<Material> m_shadowMaterial;
    };
}