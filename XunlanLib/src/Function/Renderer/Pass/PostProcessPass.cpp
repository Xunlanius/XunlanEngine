#include "PostProcessPass.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "src/Function/Resource/ConfigSystem.h"

namespace Xunlan
{
    namespace CB
    {
        struct alignas(16) PostProcess final
        {
            uint32 m_mainRTIndex;
        };
    }

    PostProcessPass::PostProcessPass()
    {
        m_canvas = RenderPassBase::CreateCanvas();
        m_cBuffer = RHI::Instance().CreateCBuffer<CB::PostProcess>();
        CreateMaterials();
    }

    void PostProcessPass::Render(Ref<RenderContext> context, PostProcessEffect effect, CRef<RenderTarget> inputRT)
    {
        CB::PostProcess* cbPostProcess = m_cBuffer->GetData<CB::PostProcess>();
        cbPostProcess->m_mainRTIndex = inputRT->GetHeapIndex();
        context->SetParam("g_postProcess", m_cBuffer);

        RHI& rhi = RHI::Instance();

        rhi.SetRT(context);
        rhi.ClearRT(context);
        rhi.SetViewport(context, 0, 0, 0, 0);

        switch (effect)
        {
        case PostProcessEffect::NONE:
        {
            m_canvas->Render(context, m_noneEffect);
            break;
        }
        default: assert(false);
        }

        rhi.ResetRT(context);
    }

    void PostProcessPass::CreateMaterials()
    {
        RHI& rhi = RHI::Instance();
        ConfigSystem& configSystem = ConfigSystem::Instance();

        const std::filesystem::path shaderPath = configSystem.GetHLSLFolder() / "PostProcessNone.hlsl";

        ShaderInitDesc shaderDesc = {};
        shaderDesc.m_createVS = true;
        shaderDesc.m_createPS = true;

        Ref<Shader> shader = rhi.CreateShader("PostProcess_None", shaderDesc, shaderPath);
        shader->GetDepthStencilState()->SetDepthEnable(false);
        shader->GetRasterizerState()->SetCullMode(CullMode::NONE);

        m_noneEffect = rhi.CreateMaterial(shader);
    }
}