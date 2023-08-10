#include "RenderPipeline.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "src/Function/World/Scene.h"

namespace Xunlan
{
    void RenderPipeline::Initialize(uint32 width, uint32 height)
    {
        m_shadowPass = std::make_unique<ShadowPass>();
        m_meshLightPass = std::make_unique<MeshLightPass>(width, height);
        m_postProcessPass = std::make_unique<PostProcessPass>();
    }
    void RenderPipeline::Shutdown()
    {
        m_shadowPass.reset();
        m_meshLightPass.reset();
        m_postProcessPass.reset();
    }

    void RenderPipeline::RenderForward()
    {
        RHI& rhi = RHI::Instance();
        Ref<RenderContext> context = rhi.CreateRenderContext();
        Scene& scene = Singleton<Scene>::Instance();

        scene.GetCBufferPerScene()->Bind(context);

        m_shadowPass->Render(context);
        m_meshLightPass->Render(context);
        m_postProcessPass->Render(context, PostProcessEffect::NONE, m_meshLightPass->GetMainRT());

        rhi.Execute(context);
        rhi.Present();
    }
}