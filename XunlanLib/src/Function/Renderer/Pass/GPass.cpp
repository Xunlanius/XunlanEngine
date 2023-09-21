#include "GPass.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "src/Function/World/Scene.h"

namespace Xunlan
{
    GPass::GPass(uint32 width, uint32 height)
        : m_width(width), m_height(height)
    {
        RHI& rhi = RHI::Instance();

        m_albedoWS = rhi.CreateRT(width, height, TextureFormat::R8G8B8A8_Unorm);
        m_posWS = rhi.CreateRT(width, height, TextureFormat::R32G32B32A32_Float);
        m_normalWS = rhi.CreateRT(width, height, TextureFormat::R16G16B16A16_Snorm);
        m_depth = rhi.CreateDepthBuffer(width, height);
    }

    void GPass::Render(Ref<RenderContext> context, const std::vector<Ref<RenderItem>>& items)
    {
        RHI& rhi = RHI::Instance();

        std::vector<CRef<RenderTarget>> rts = {
            m_albedoWS,
            m_posWS,
            m_normalWS
        };

        rhi.SetRT(context, rts, m_depth);
        rhi.ClearRT(context, rts, m_depth);
        rhi.SetViewport(context, 0, 0, m_width, m_height);

        RenderItems(context, items);

        rhi.ResetRT(context, rts, m_depth);
    }

    void GPass::RenderItems(Ref<RenderContext> context, const std::vector<Ref<RenderItem>>& items)
    {
        for (const Ref<RenderItem>& item : items)
        {
            assert(item);

            context->SetParam("g_perObject", item->GetPerObject());

            item->Render(context);
        }
    }
}