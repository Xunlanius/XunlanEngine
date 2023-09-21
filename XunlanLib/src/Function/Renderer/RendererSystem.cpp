#include "RendererSystem.h"
#include "src/Function/Renderer/DX12/DX12RHI.h"
#include "src/Function/World/Component/Transformer.h"
#include "src/Function/World/Component/Camera.h"
#include "src/Function/World/Scene.h"

namespace Xunlan
{
    bool RenderSystem::Initialize(Platform platform)
    {
        WindowSystem& windowSystem = WindowSystem::Instance();

        // Initialize RHI
        RHI::Create(platform);

        RenderPipeline& renderPipeline = RenderPipeline::Instance();
        renderPipeline.Initialize(windowSystem.GetWidth(), windowSystem.GetHeight());

        return true;
    }
    void RenderSystem::Shutdown()
    {
        RenderPipeline::Instance().Shutdown();
        RHI::Shutdown();
    }
    void RenderSystem::Render(float deltaTime)
    {
        RenderPipeline::Instance().RenderForward();
    }

    uint32 RenderSystem::GetWidth() const { return RHI::Instance().GetWidth(); }
    uint32 RenderSystem::GetHeight() const { return RHI::Instance().GetHeight(); }
}