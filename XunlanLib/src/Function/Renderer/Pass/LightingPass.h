#pragma once

#include "src/Common/Common.h"
#include "RenderPassBase.h"
#include "src/Function/World/Entity.h"
#include "src/Function/Renderer/Abstract/Texture.h"
#include "src/Function/Renderer/Abstract/RasterizerState.h"
#include "src/Function/Renderer/Abstract/RenderItem.h"

namespace Xunlan
{
    class LightingPass final : public RenderPassBase
    {
    public:

        explicit LightingPass(uint32 width, uint32 height);

    public:

        void Render(
            Ref<RenderContext> context,
            CRef<RenderTarget> albedo,
            CRef<RenderTarget> posWS,
            CRef<RenderTarget> normalWS,
            CRef<RenderTarget> rsm,
            CRef<DepthBuffer> shadowMap);

        Ref<RenderTarget> GetMainRT() const { return m_mainRT; }

    private:

        void CreateMaterial();

        uint32 m_width = 0;
        uint32 m_height = 0;

        Ref<RenderTarget> m_mainRT;

        Ref<RenderItem> m_canvas;
        Ref<CBuffer> m_cBuffer;
        Ref<Material> m_lighting;
    };
}