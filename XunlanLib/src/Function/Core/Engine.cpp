#include "Engine.h"
#include "RuntimeContext.h"
#include "src/Function/Renderer/WindowSystem.h"
#include "src/Function/Renderer/RendererSystem.h"
#include "src/Function/World/Component/Component.h"
#include <thread>

namespace Xunlan
{
    bool EngineSystem::Initialize()
    {
        if (!g_runtimeContext.Start()) return false;
        m_running = true;
        return true;
    }
    void EngineSystem::OnTick()
    {
        //m_timer.Begin();

        std::this_thread::sleep_for(std::chrono::microseconds(10));
        OnTickLogical(10.0f);
        OnTickRender(10.0f);

        //m_timer.End();
    }
    void EngineSystem::Shutdown()
    {
        g_runtimeContext.Shutdown();
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

    void EngineSystem::OnTickLogical(float deltaTime)
    {
        ECS::ECSManager& instance = Singleton<ECS::ECSManager>::Instance();

        CameraSystem& cameraSystem = instance.GetSystem<CameraSystem>();
        ScriptSystem& scriptSystem = instance.GetSystem<ScriptSystem>();

        cameraSystem.OnUpdate();
        scriptSystem.OnUpdate(10.0f);
    }
    void EngineSystem::OnTickRender(float deltaTime)
    {
        g_runtimeContext.m_renderSystem->Render(deltaTime);
    }
}