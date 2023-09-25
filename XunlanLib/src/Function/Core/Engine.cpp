#include "Engine.h"
#include "src/Tool/Geometry/GeometrySystem.h"
#include "src/Function/World/Scene.h"
#include "src/Function/Renderer/WindowSystem.h"
#include "src/Function/Renderer/RendererSystem.h"
#include "src/Function/World/Component/Component.h"
#include <thread>

namespace Xunlan
{
    bool EngineSystem::Initialize()
    {
        RegisterECS();

        GeometryImportSystem& geometryImportSystem = GeometryImportSystem::Instance();
        if (!geometryImportSystem.Initialize()) return false;

        WindowInitDesc windowInitDesc = {};
        windowInitDesc.caption = L"Xunlan Game";
        windowInitDesc.isCenter = true;

        WindowSystem& windowSystem = WindowSystem::Instance();
        if (!windowSystem.Initialize(windowInitDesc)) return false;

        RenderSystem& renderSystem = RenderSystem::Instance();
        if (!renderSystem.Initialize(Platform::DX12)) return false;

        Scene& scene = Scene::Instance();
        scene.Initialize();
        scene.LoadScene();
        scene.OnScenePlay();

        return true;
    }
    void EngineSystem::Shutdown()
    {
        Scene& scene = Scene::Instance();
        RenderSystem& renderSystem = RenderSystem::Instance();

        scene.OnSceneDestroy();
        scene.Shutdown();
        renderSystem.Shutdown();
    }
    void EngineSystem::Tick()
    {
        //m_timer.Begin();

        std::this_thread::sleep_for(std::chrono::microseconds(10));
        TickLogical(10.0f);
        TickRender(10.0f);

        //m_timer.End();
    }
    bool EngineSystem::IsRunning()
    {
        static MSG msg = {};

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) return false;
        }

        return true;
    }

    void EngineSystem::TickLogical(float deltaTime)
    {
        CameraSystem::Update();
        ScriptSystem::Update(10.0f);
        MeshRenderSystem::Update();
    }
    void EngineSystem::TickRender(float deltaTime)
    {
        RenderSystem::Instance().Render(deltaTime);
    }
}