#include "Engine.h"
#include "Platforms/Window.h"
#include "Renderer/Renderer.h"
#include "Content/ContentLoader.h"

#include <thread>

using namespace Xunlan;

namespace
{
    Graphics::RenderSurface g_renderSurface = {};

    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        ID windowID = g_renderSurface.windowID;

        switch (msg)
        {
        case WM_DESTROY:
        {
            if (Graphics::Window::IsClosed(windowID))
            {
                ::PostQuitMessage(0);
                return 0;
            }
            break;
        }

        case WM_SYSCHAR:
            if (wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN))
            {
                Graphics::Window::SetFullScreen(windowID, !Graphics::Window::IsFullScreen(windowID));
                return 0;
            }
            break;
        }

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool EngineInitialize()
{
    if (!ContentLoader::LoadGame()) return false;
    if (!Graphics::Initialize(Graphics::Platform::DX12)) return false;

    Graphics::WindowInitDesc desc = {};
    desc.callback = WndProc;
    desc.caption = L"Xunlan Game";
    desc.isCenter = true;

    g_renderSurface.windowID = Graphics::Window::Create(&desc);
    g_renderSurface.surfaceID = Graphics::Surface::Create(g_renderSurface.windowID);

    if (!IsValid(g_renderSurface.windowID) || !IsValid(g_renderSurface.surfaceID)) return false;

    RegisterECS();

    return true;
}
void EngineOnTick()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    Script::ScriptSystem* scriptSystem = ECS::GetSystem<Script::ScriptSystem>();
    scriptSystem->OnUpdate(10.0f);

    Graphics::Surface::Render(g_renderSurface.surfaceID);
}
void EngineShutdown()
{
    Graphics::Surface::Remove(g_renderSurface.surfaceID);
    Graphics::Window::Remove(g_renderSurface.windowID);
    Graphics::Shutdown();
    ContentLoader::UnloadGame();
}