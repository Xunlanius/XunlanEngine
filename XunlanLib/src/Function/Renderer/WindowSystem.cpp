#include "WindowSystem.h"
#include "src/Function/Input/platform_win32/Win32Input.h"
#include <cassert>
#include <string>

namespace Xunlan
{
#if defined _WIN64

    bool WindowSystem::Initialize(const WindowInitDesc& initDesc)
    {
        // Setup
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = 0;
        wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = ::CreateSolidBrush(RGB(240, 248, 255));
        wc.lpszMenuName = NULL;
        wc.lpszClassName = L"XunlanWindow";
        wc.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);

        // Register
        ::RegisterClassEx(&wc);

        int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

        m_windowInfo.width = initDesc.width;
        m_windowInfo.height = initDesc.height;
        m_windowInfo.style |= initDesc.parent ? WS_CHILD : WS_OVERLAPPEDWINDOW;

        // Adjust the size of the windowID
        RECT windowArea = { 0, 0, (LONG)m_windowInfo.width, (LONG)m_windowInfo.height };
        ::AdjustWindowRect(&windowArea, m_windowInfo.style, FALSE);

        const int32 wndWidth = windowArea.right - windowArea.left;
        const int32 wndHeight = windowArea.bottom - windowArea.top;

        m_windowInfo.leftTop.x = !initDesc.isCenter ? initDesc.left : (screenWidth - wndWidth) / 2;
        m_windowInfo.leftTop.y = !initDesc.isCenter ? initDesc.top : (screenHeight - wndHeight) / 2;

        const wchar_t* caption = initDesc.caption ? initDesc.caption : L"Xunlan Game";
        const int32 wndLeft = m_windowInfo.leftTop.x;
        const int32 wndTop = m_windowInfo.leftTop.y;

        // Create instance of the windowID
        m_windowInfo.hwnd = ::CreateWindowEx(
            0,                  // DWORD        dwExStyle
            wc.lpszClassName,   // LPCWSTR      lpClassName
            caption,            // LPCWSTR      lpWindowName
            m_windowInfo.style, // DWORD        dwStyle
            wndLeft,            // int          X
            wndTop,             // int          Y
            wndWidth,           // int          nWidth
            wndHeight,          // int          nHeight
            initDesc.parent,    // HWND         hWndParent
            NULL,               // HMENU        hMenu
            NULL,               // HINSTANCE    hInstance
            NULL                // LPVOID       lpParam
        );

        if (!m_windowInfo.hwnd) { assert(false); return false; }

        ::SetWindowLongPtr(m_windowInfo.hwnd, GWLP_USERDATA, (LONG_PTR)this);

        ::ShowWindow(m_windowInfo.hwnd, SW_SHOWNORMAL);
        ::UpdateWindow(m_windowInfo.hwnd);

        return true;
    }
    void WindowSystem::Shutdown() { ::DestroyWindow(m_windowInfo.hwnd); }

    void* WindowSystem::GetHandle() const { return m_windowInfo.hwnd; }

    void WindowSystem::SetCaption(const wchar_t* caption) { SetWindowText(m_windowInfo.hwnd, caption); }

    uint32 WindowSystem::GetWidth() const { return m_windowInfo.width; }
    uint32 WindowSystem::GetHeight() const { return m_windowInfo.height; }

    LRESULT WindowSystem::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
            case WM_DESTROY:
            {
                ::PostQuitMessage(0);
                Dispatch(hwnd, &OnClosed);
                return 0;
            }
            case WM_SIZE:
            {
                uint32 width = LOWORD(lParam);
                uint32 height = HIWORD(lParam);
                Dispatch(hwnd, &OnResized, width, height);
                break;
            }
            case WM_MOVE:
            {
                int x = (int)(short)LOWORD(lParam);
                int y = (int)(short)HIWORD(lParam);
                Dispatch(hwnd, &OnMoved, x, y);
                break;
            }
            default: break;
        }

        ProcessInputMessage(hwnd, msg, wParam, lParam);

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

    void WindowSystem::OnResized(uint32 width, uint32 height)
    {
        m_windowInfo.width = width;
        m_windowInfo.height = height;

        for (OnResizeFunc func : m_onResizeFuncs)
        {
            func(width, height);
        }
    }
    void WindowSystem::OnMoved(int x, int y)
    {
        m_windowInfo.leftTop.x = x;
        m_windowInfo.leftTop.y = y;

        for (OnMoveFunc func : m_onMoveFuncs)
        {
            func(x, y);
        }
    }
    void WindowSystem::OnClosed()
    {
        for (OnClosedFunc func : m_onClosedFuncs)
        {
            func();
        }
    }

#else
    #error "Must implement at least one platform"
#endif
}