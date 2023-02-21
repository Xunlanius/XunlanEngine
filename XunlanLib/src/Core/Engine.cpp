#include "EngineAPI/Script.h"
#include "Platforms/WindowManager.h"
#include "Renderer/Renderer.h"
#include "Content/ContentLoader.h"

using namespace Xunlan;

namespace
{
    RenderSurface g_gameWindow = {};

    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Window window = g_gameWindow.window;

        switch (msg)
        {
        case WM_DESTROY:
        {
            if (WindowManager::IsClosed(window))
            {
                ::PostQuitMessage(0);
                return 0;
            }

            break;
        }

        case WM_SYSCHAR:
            if (wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN))
            {
                WindowManager::SetFullScreen(window, !WindowManager::IsFullScreen(window));
                return 0;
            }
            break;
        }

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool EngineInitialize()
{
    bool result = ContentLoader::LoadGame();

    if (!result)
    {
        assert(false);
        return false;
    }

    WindowInitDesc desc = {};
    desc.callback = WndProc;
    desc.caption = L"Xunlan Game";

    g_gameWindow.window = WindowManager::CreateWnd(&desc);
    if (!g_gameWindow.window.IsValid()) return false;

    return true;
}

void EngineOnTick()
{
    Script::OnUpdate(10.0f);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void EngineShutdown()
{
    WindowManager::RemoveWnd(g_gameWindow.window);
    ContentLoader::UnloadGame();
}