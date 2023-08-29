#include "LightingPass.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "src/Function/World/Scene.h"
#include "src/Function/World/Component/MeshRender.h"
#include "src/Function/Resource/ConfigSystem.h"

namespace Xunlan
{
    MeshLightPass::MeshLightPass(uint32 width, uint32 height)
        : m_width(width), m_height(height)
    {
        m_mainRT = RHI::Instance().CreateRT(width, height);
        m_canvas = RenderPassBase::CreateCanvas();
        m_lighting = CreateMaterial();
    }

    void MeshLightPass::Render(Ref<RenderContext> context)
    {
        RHI& rhi = RHI::Instance();

        std::vector<CRef<RenderTarget>> rts = { m_mainRT };

        rhi.SetRT(context, rts);
        rhi.ClearRT(context, rts);
        rhi.SetViewport(context, 0, 0, m_width, m_height);

        m_canvas->Render(context, m_lighting);

        rhi.ResetRT(context, rts);
    }

    Ref<Material> MeshLightPass::CreateMaterial()
    {
        RHI& rhi = RHI::Instance();
        ConfigSystem& configSystem = Singleton<ConfigSystem>::Instance();

        const std::filesystem::path shaderPath = configSystem.GetHLSLFolder() / "Lighting.hlsl";

        ShaderList list = {};
        list.m_VS = rhi.CreateShader(ShaderType::VERTEX_SHADER, shaderPath, "VS");
        list.m_PS = rhi.CreateShader(ShaderType::PIXEL_SHADER, shaderPath, "PS");

        Ref<Material> lighting = rhi.CreateMaterial("Mat_lighting", MaterialType::PostProcess, list);
        lighting->GetDepthStencilState()->SetDepthEnable(false);
        lighting->GetRasterizerState()->SetCullMode(CullMode::NONE);

        return lighting;
    }
}