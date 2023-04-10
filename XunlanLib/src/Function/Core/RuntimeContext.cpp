#include "RuntimeContext.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Tool/Geometry/GeometrySystem.h"
#include "src/Function/World/Scene.h"
#include "src/Function/Renderer/RenderCommon.h"
#include "src/Function/Renderer/WindowSystem.h"
#include "src/Function/Renderer/RendererSystem.h"
#include "src/Function/World/Component/Component.h"

namespace Xunlan
{
    bool RuntimeContext::Start()
    {
        RegisterECS();

        m_geometryImportSystem = std::make_shared<GeometryImportSystem>();
        m_geometryImportSystem->Initialize();

        WindowInitDesc windowInitDesc = {};
        windowInitDesc.caption = L"Xunlan Game";
        windowInitDesc.isCenter = true;

        m_windowSystem = std::make_shared<WindowSystem>();
        if (!m_windowSystem->Initialize(windowInitDesc)) return false;

        RenderSystemInitDesc renderSystemInitDesc = {};
        renderSystemInitDesc.windowSystem = m_windowSystem;

        m_renderSystem = std::make_shared<RenderSystem>();
        if (!m_renderSystem->Initialize(Platform::DX12, renderSystemInitDesc)) return false;

        Scene& scene = Singleton<Scene>::Instance();
        scene.Initialize();
        scene.LoadScene();

        return true;
    }
    void RuntimeContext::Shutdown()
    {
        Singleton<Scene>::Instance().Shutdown();

        m_renderSystem->Shutdown();
        m_renderSystem.reset();
    }
}