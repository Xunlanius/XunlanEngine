#include "RendererSystem.h"
#include "src/Function/Renderer/DX12/DX12RHI.h"
#include "src/Function/World/Component/Transformer.h"
#include "src/Function/World/Component/Camera.h"
#include "src/Function/World/Scene.h"

namespace Xunlan
{
    bool RenderSystem::Initialize(Platform platform)
    {
        WindowSystem& windowSystem = Singleton<WindowSystem>::Instance();

        // Initialize RHI
        RHI::Create(platform);

        RenderPipeline& renderPipeline = Singleton<RenderPipeline>::Instance();
        renderPipeline.Initialize(windowSystem.GetWidth(), windowSystem.GetHeight());

        return true;
    }
    void RenderSystem::Shutdown()
    {
        Singleton<RenderPipeline>::Instance().Shutdown();
        RHI::Shutdown();
    }
    void RenderSystem::Render(float deltaTime)
    {
        Singleton<Scene>::Instance().UpdateCBufferPerScene();
        Singleton<RenderPipeline>::Instance().RenderForward();
    }

    uint32 RenderSystem::GetWidth() const { return RHI::Instance().GetWidth(); }
    uint32 RenderSystem::GetHeight() const { return RHI::Instance().GetHeight(); }
}