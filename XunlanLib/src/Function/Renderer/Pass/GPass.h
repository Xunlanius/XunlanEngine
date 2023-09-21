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

        void Render(Ref<RenderContext> context, const std::vector<Ref<RenderItem>>& items);

        CRef<RenderTarget> GetAlbedoWS() const { return m_albedoWS; }
        CRef<RenderTarget> GetPosWS() const { return m_posWS; }
        CRef<RenderTarget> GetNormalWS() const { return m_normalWS; }

    private:

        void RenderItems(Ref<RenderContext> context, const std::vector<Ref<RenderItem>>& items);

    private:

        uint32 m_width;
        uint32 m_height;

        Ref<RenderTarget> m_albedoWS;
        Ref<RenderTarget> m_posWS;
        Ref<RenderTarget> m_normalWS;
        Ref<DepthBuffer> m_depth;
    };
}