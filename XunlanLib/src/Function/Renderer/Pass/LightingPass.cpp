#include "LightingPass.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Function/Renderer/Abstract/RHI.h"

namespace Xunlan
{
    namespace CB
    {
        struct alignas(16) ShadowMapAndGBuffer final
        {
            uint32 m_albedoIndex;
            uint32 m_posWSIndex;
            uint32 m_normalWSIndex;

            uint32 m_rsmIndex;
            uint32 m_shadowMapIndex;
        };
    }

    LightingPass::LightingPass(uint32 width, uint32 height)
        : m_width(width), m_height(height)
    {
        RHI& rhi = RHI::Instance();

        m_mainRT = rhi.CreateRT(width, height, TextureFormat::R8G8B8A8_Unorm);
        m_canvas = RenderPassBase::CreateCanvas();
        m_cBuffer = rhi.CreateCBuffer<CB::ShadowMapAndGBuffer>();

        CreateMaterial();
    }

    void LightingPass::Render(
        Ref<RenderContext> context,
        CRef<RenderTarget> albedo,
        CRef<RenderTarget> posWS,
        CRef<RenderTarget> normalWS,
        CRef<RenderTarget> rsm,
        CRef<DepthBuffer> shadowMap)
    {
        CB::ShadowMapAndGBuffer* cbPerMaterial = m_cBuffer->GetData<CB::ShadowMapAndGBuffer>();
        cbPerMaterial->m_albedoIndex = albedo->GetHeapIndex();
        cbPerMaterial->m_posWSIndex = posWS->GetHeapIndex();
        cbPerMaterial->m_normalWSIndex = normalWS->GetHeapIndex();
        cbPerMaterial->m_rsmIndex = rsm->GetHeapIndex();
        cbPerMaterial->m_shadowMapIndex = shadowMap->GetHeapIndex();
        context->SetParam("g_lightingPass", m_cBuffer);

        RHI& rhi = RHI::Instance();

        std::vector<CRef<RenderTarget>> rts = { m_mainRT };

        rhi.SetRT(context, rts);
        rhi.ClearRT(context, rts);
        rhi.SetViewport(context, 0, 0, m_width, m_height);

        m_canvas->Render(context, m_lighting);

        rhi.ResetRT(context, rts);
    }

    void LightingPass::CreateMaterial()
    {
        RHI& rhi = RHI::Instance();
        ConfigSystem& configSystem = ConfigSystem::Instance();

        const std::filesystem::path shaderPath = configSystem.GetHLSLFolder() / "Lighting.hlsl";

        ShaderInitDesc shaderDesc = {};
        shaderDesc.m_createVS = true;
        shaderDesc.m_createPS = true;

        Ref<Shader> shader = rhi.CreateShader("Lighting", shaderDesc, shaderPath);
        shader->GetDepthStencilState()->SetDepthEnable(false);
        shader->GetRasterizerState()->SetCullMode(CullMode::NONE);

        m_lighting = rhi.CreateMaterial(shader);
    }
}