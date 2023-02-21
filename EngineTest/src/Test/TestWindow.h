#pragma once

#include "ITest.h"
#include "Platforms/PlatformsType.h"
#include "Platforms/WindowManager.h"

namespace
{
    Xunlan::Window g_windows[2];
}

class TestWindow : public ITest
{
public:

    virtual bool Initialize() override
    {
        Xunlan::WindowInitDesc initDesc = {};
        initDesc.callback = WndProc;
        initDesc.caption = L"Test Window";
        initDesc.left = 100;
        initDesc.top = 200;
        initDesc.width = 800;
        initDesc.height = 400;

        for (size_t i = 0; i < _countof(g_windows); ++i)
        {
            g_windows[i] = Xunlan::WindowManager::CreateWnd();
        }

        return true;
    }
    virtual void Run() override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    virtual void Shutdown() override
    {
        for (size_t i = 0; i < _countof(g_windows); ++i)
        {
            Xunlan::WindowManager::RemoveWnd(g_windows[i]);
        }
    }

private:

    static LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
            case WM_DESTROY:
            {
                bool allClosed = true;

                for (size_t i = 0; i < _countof(g_windows); ++i)
                {
                    if (!Xunlan::WindowManager::IsClosed(g_windows[i])) allClosed = false;
                }

                if (allClosed)
                {
                    ::PostQuitMessage(0);
                    return 0;
                }

                break;
            }

            case WM_SYSCHAR:
                if (wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN))
                {
                    Xunlan::Window wnd((Xunlan::ID::id_type)::GetWindowLongPtr(hwnd, GWLP_USERDATA));
                    Xunlan::WindowManager::SetFullScreen(wnd, !Xunlan::WindowManager::IsFullScreen(wnd));
                    return 0;
                }
                break;
        }

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }
};