#include "PostProcessPass.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "src/Function/Resource/ConfigSystem.h"

namespace Xunlan
{
    PostProcessPass::PostProcessPass()
    {
        CreateMaterials();
        m_canvas = RenderPassBase::CreateCanvas();
    }

    void PostProcessPass::Render(Ref<RenderContext> context, PostProcessEffect effect, CRef<RenderTarget> inputRT)
    {
        RHI& rhi = RHI::Instance();

        rhi.SetRT(context);
        rhi.ClearRT(context);
        rhi.SetViewport(context, 0, 0, 0, 0);

        switch (effect)
        {
        case PostProcessEffect::NONE:
        {
            m_noneEffect->SetTexture(TextureCategory::Albedo, inputRT);
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

        const std::filesystem::path noneEffectShaderPath = configSystem.GetHLSLFolder() / "PostProcessNone.hlsl";

        ShaderList list = {};
        list.m_VS = rhi.CreateShader(ShaderType::VERTEX_SHADER, noneEffectShaderPath, "VS");
        list.m_PS = rhi.CreateShader(ShaderType::PIXEL_SHADER, noneEffectShaderPath, "PS");

        m_noneEffect = rhi.CreateMaterial("Mat_PostProcess_None", MaterialType::PostProcess, list);
        m_noneEffect->GetDepthStencilState()->SetDepthEnable(false);
        m_noneEffect->GetRasterizerState()->SetCullMode(CullMode::NONE);
    }
}