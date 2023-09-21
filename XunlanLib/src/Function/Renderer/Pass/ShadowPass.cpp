#include "ShadowPass.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Function/World/Scene.h"
#include "src/Function/World/Component/MeshRender.h"

namespace Xunlan
{
    ShadowPass::ShadowPass(uint32 width, uint32 height)
        : m_width(width), m_height(height)
    {
        RHI& rhi = RHI::Instance();

        m_flux = rhi.CreateRT(width, height, TextureFormat::R8G8B8A8_Unorm);
        m_posWS = rhi.CreateRT(width, height, TextureFormat::R32G32B32A32_Float);
        m_normalWS = rhi.CreateRT(width, height, TextureFormat::R16G16B16A16_Snorm);
        m_depth = rhi.CreateDepthBuffer(width, height);

        CreateMaterial();
    }

    void ShadowPass::Render(Ref<RenderContext> context, const std::vector<Ref<RenderItem>>& items)
    {
        RHI& rhi = RHI::Instance();

        std::vector<CRef<RenderTarget>> rts = {
            m_flux,
            m_posWS,
            m_normalWS
        };

        rhi.SetRT(context, rts, m_depth);
        rhi.ClearRT(context, rts, m_depth);
        rhi.SetViewport(context, 0, 0, m_width, m_height);

        RenderItems(context, items);

        rhi.ResetRT(context, rts, m_depth);
    }

    void ShadowPass::CreateMaterial()
    {
        RHI& rhi = RHI::Instance();
        ConfigSystem& configSystem = ConfigSystem::Instance();

        const std::filesystem::path shaderPath = configSystem.GetHLSLFolder() / "ShadowMapping.hlsl";

        ShaderInitDesc shaderDesc = {};
        shaderDesc.m_createVS = true;
        shaderDesc.m_createPS = true;

        Ref<Shader> shader = rhi.CreateShader("Shadow_Mapping", shaderDesc, shaderPath);
        shader->GetRasterizerState()->SetDepthClipEnable(false);

        m_shadowMaterial = rhi.CreateMaterial(shader);
    }

    void ShadowPass::RenderItems(Ref<RenderContext> context, const std::vector<Ref<RenderItem>>& items)
    {
        for (const Ref<RenderItem>& item : items)
        {
            assert(item);

            context->SetParam("g_perObject", item->GetPerObject());
            item->Render(context, m_shadowMaterial);
        }
    }
}