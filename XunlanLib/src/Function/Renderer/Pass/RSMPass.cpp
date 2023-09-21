#include "RSMPass.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Function/Renderer/Abstract/RHI.h"

#include <random>

namespace Xunlan
{
    namespace CB
    {
        static constexpr uint32 RSM_SAMPLES_COUNT = 128;

        struct alignas(16) ShadowMap final
        {
            uint32 m_fluxIndex;
            uint32 m_posWSIndex;
            uint32 m_normalWSIndex;
            uint32 m_pad0;
        };
        
        struct alignas(16) ShadowMapAndGBuffer final
        {
            uint32 m_gbPosWSIndex;
            uint32 m_gbNormalWSIndex;

            ShadowMap m_shadowMap;

            Math::float4 m_xi[RSM_SAMPLES_COUNT];
        };
    }

    RSMPass::RSMPass(uint32 width, uint32 height)
    {
        RHI& rhi = RHI::Instance();

        m_mainRT = rhi.CreateRT(width, height, TextureFormat::R8G8B8A8_Unorm);
        m_canvas = RenderPassBase::CreateCanvas();
        m_cBuffer = rhi.CreateCBuffer<CB::ShadowMapAndGBuffer>();

        CreateMaterial();
    }

    void RSMPass::Render(
        Ref<RenderContext> context,
        CRef<RenderTarget> posWS,
        CRef<RenderTarget> normalWS,
        CRef<RenderTarget> flux,
        CRef<RenderTarget> posWSLight,
        CRef<RenderTarget> normalWSLight)
    {
        CB::ShadowMapAndGBuffer* cbRSMPass = m_cBuffer->GetData<CB::ShadowMapAndGBuffer>();
        cbRSMPass->m_gbPosWSIndex = posWS->GetHeapIndex();
        cbRSMPass->m_gbNormalWSIndex = normalWS->GetHeapIndex();
        cbRSMPass->m_shadowMap.m_fluxIndex = flux->GetHeapIndex();
        cbRSMPass->m_shadowMap.m_posWSIndex = posWSLight->GetHeapIndex();
        cbRSMPass->m_shadowMap.m_normalWSIndex = normalWSLight->GetHeapIndex();
        context->SetParam("g_rsmPass", m_cBuffer);

        std::default_random_engine engine;
        std::uniform_real_distribution<float> u(0, 1);

        for (uint32 i = 0; i < CB::RSM_SAMPLES_COUNT; ++i)
        {
            cbRSMPass->m_xi[i].x = u(engine);
            cbRSMPass->m_xi[i].y = u(engine);
        }

        RHI& rhi = RHI::Instance();

        std::vector<CRef<RenderTarget>> rts = { m_mainRT };

        rhi.SetRT(context, rts);
        rhi.ClearRT(context, rts);
        rhi.SetViewport(context, 0, 0, m_width, m_height);

        m_canvas->Render(context, m_material);

        rhi.ResetRT(context, rts);
    }

    void RSMPass::CreateMaterial()
    {
        RHI& rhi = RHI::Instance();
        ConfigSystem& configSystem = ConfigSystem::Instance();

        const std::filesystem::path shaderPath = configSystem.GetHLSLFolder() / "RSM.hlsl";

        ShaderInitDesc shaderDesc = {};
        shaderDesc.m_createVS = true;
        shaderDesc.m_createPS = true;

        Ref<Shader> shader = rhi.CreateShader("RSM", shaderDesc, shaderPath);
        shader->GetDepthStencilState()->SetDepthEnable(false);
        shader->GetRasterizerState()->SetCullMode(CullMode::NONE);

        m_material = rhi.CreateMaterial(shader);
    }
}