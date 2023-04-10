#include "RendererSystem.h"
#include "src/Function/Renderer/DX12/DX12RHI.h"
#include "src/Function/World/Component/Transformer.h"
#include "src/Function/World/Component/Camera.h"
#include "src/Function/World/Scene.h"
#include "src/Function/Core/RuntimeContext.h"

namespace Xunlan
{
    bool RenderSystem::Initialize(Platform platform, const RenderSystemInitDesc& initDesc)
    {
        m_windowSystem = initDesc.windowSystem;

        RHIInitDesc rhiInitDesc = {};
        rhiInitDesc.windowSystem = m_windowSystem;

        // Initialize RHI
        RHI::Create(platform, rhiInitDesc);

        Singleton<RenderPipeline>::Instance().Initialize(m_windowSystem->GetWidth(), m_windowSystem->GetHeight());

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