#pragma once

#include "src/Common/Common.h"
#include "RenderPassBase.h"
#include "src/Function/World/Entity.h"
#include "src/Function/Renderer/Abstract/Texture.h"
#include "src/Function/Renderer/Abstract/RasterizerState.h"
#include "src/Function/Renderer/Abstract/RenderItem.h"

namespace Xunlan
{
    class RSMPass final : public RenderPassBase
    {
    public:

        explicit RSMPass(uint32 width, uint32 height);

    public:

        void Render(
            Ref<RenderContext> context,
            CRef<RenderTarget> posWS,
            CRef<RenderTarget> normalWS,
            CRef<RenderTarget> flux,
            CRef<RenderTarget> posWSLight,
            CRef<RenderTarget> normalWSLight);

        Ref<RenderTarget> GetRSM() const { return m_mainRT; }

    private:

        void CreateMaterial();

        uint32 m_width = 0;
        uint32 m_height = 0;

        Ref<RenderTarget> m_mainRT;

        Ref<RenderItem> m_canvas;
        Ref<CBuffer> m_cBuffer;
        Ref<Material> m_material;
    };
}