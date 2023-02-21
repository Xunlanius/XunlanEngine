#include "TestRenderer.h"
#include "Platforms/PlatformsType.h"
#include "Platforms/Window.h"
#include "Renderer/Renderer.h"
#include "ShaderCompilation.h"

#include <thread>

using namespace Xunlan;

namespace
{
    Graphics::RenderSurface g_renderSurfaces[1] = {};

    Timer g_timer = {};
    bool g_restarting = false;
    bool g_resizing = false;

    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void CreateRenderSurface(Graphics::RenderSurface& renderSurface, const Graphics::WindowInitDesc& initDesc)
    {
        renderSurface.windowID = Graphics::Window::Create(&initDesc);
        renderSurface.surfaceID = Graphics::Surface::Create(renderSurface.windowID);
    }
    void RemoveRenderSurface(Graphics::RenderSurface& renderSurface)
    {
        if (!ID::IsValid(renderSurface.windowID) || !ID::IsValid(renderSurface.surfaceID)) return;

        Graphics::Surface::Remove(renderSurface.surfaceID);
        Graphics::Window::Remove(renderSurface.windowID);
    }

    bool TestInitialize()
    {
        if (!CompileShader()) return false;

        if (!Graphics::Initialize(Graphics::Platform::DX12)) return false;

        for (size_t i = 0; i < _countof(g_renderSurfaces); ++i)
        {
            Graphics::WindowInitDesc initDesc = {};
            initDesc.callback = WndProc;
            initDesc.caption = L"Test Renderer";
            initDesc.isCenter = true;
            initDesc.width = 1280;
            initDesc.height = 720;

            CreateRenderSurface(g_renderSurfaces[i], initDesc);
        }

        return true;
    }
    void TestShutdown()
    {
        for (size_t i = 0; i < _countof(g_renderSurfaces); ++i)
        {
            RemoveRenderSurface(g_renderSurfaces[i]);
        }

        Graphics::Shutdown();
    }

    void TestRestart()
    {
        g_restarting = true;
        TestShutdown();
        TestInitialize();
        g_restarting = false;
    }
    void TestResize(HWND hwnd)
    {
        EntityID windowID = (EntityID)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

        for (uint32 i = 0; i < _countof(g_renderSurfaces); ++i)
        {
            if (g_renderSurfaces[i].windowID == windowID)
            {
                EntityID surfaceID = g_renderSurfaces[i].surfaceID;
                Graphics::Surface::Resize(surfaceID);
                g_resizing = false;
                break;
            }
        }
    }

    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_DESTROY:
        {
            bool allClosed = true;

            for (size_t i = 0; i < _countof(g_renderSurfaces); ++i)
            {
                if (!ID::IsValid(g_renderSurfaces[i].windowID)) continue;

                if (Graphics::Window::IsClosed(g_renderSurfaces[i].windowID)) RemoveRenderSurface(g_renderSurfaces[i]);
                else allClosed = false;
            }

            if (allClosed && !g_restarting)
            {
                ::PostQuitMessage(0);
                return 0;
            }

            break;
        }

        case WM_SIZE:
            g_resizing = wParam != SIZE_MINIMIZED;
            if (wParam == SIZE_MAXIMIZED) TestResize(hwnd);
            break;

        case WM_EXITSIZEMOVE:
            if (g_resizing) TestResize(hwnd);
            break;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                ::PostMessage(hwnd, WM_CLOSE, 0, 0);
                return 0;
            }
            else if (wParam == VK_F11)
            {
                TestRestart();
                return 0;
            }

            break;

        case WM_SYSCHAR:
            if (wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN))
            {
                EntityID windowID = (EntityID)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
                Graphics::Window::SetFullScreen(windowID, !Graphics::Window::IsFullScreen(windowID));
                return 0;
            }
            break;
        }

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool TestRenderer::Initialize() { return TestInitialize(); }
void TestRenderer::Run()
{
    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    g_timer.Begin();

    for (size_t i = 0; i < _countof(g_renderSurfaces); ++i)
    {
        if (ID::IsValid(g_renderSurfaces[i].surfaceID))
        {
            Graphics::Surface::Render(g_renderSurfaces[i].surfaceID);
        }
    }

    g_timer.End();
}
void TestRenderer::Shutdown() { TestShutdown(); }