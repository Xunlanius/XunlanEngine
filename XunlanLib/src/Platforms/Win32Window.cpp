#include "Window.h"
#include "Utility/Pool.h"

namespace Xunlan::Graphics
{
#if defined _WIN64

    namespace
    {
        struct WindowInfo
        {
            HWND hwnd = nullptr;
            POINT leftTop = { 0, 0 };
            RECT clientArea = { 0, 0, 1280, 720 };
            RECT fullScreenArea = {};
            DWORD style = WS_VISIBLE;
            bool isFullScreen = false;
            bool isClosed = false;
        };

        Utility::ObjectPool<WindowInfo> g_windows;

        WindowInfo& GetInfoFromID(ID windowID) { return g_windows.Get(windowID); }
        WindowInfo& GetInfoFromHandle(HWND handle)
        {
            const ID windowID = (ID)::GetWindowLongPtr(handle, GWLP_USERDATA);
            return GetInfoFromID(windowID);
        }

        // Update "WindowInfo"
        LRESULT CALLBACK InternalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            WindowInfo* pInfo = nullptr;

            switch (msg)
            {
                case WM_DESTROY:
                    GetInfoFromHandle(hwnd).isClosed = true;
                    break;

                case WM_EXITSIZEMOVE:
                    pInfo = &GetInfoFromHandle(hwnd);
                    break;

                case WM_SIZE:
                    if (wParam == SIZE_MAXIMIZED) pInfo = &GetInfoFromHandle(hwnd);
                    break;

                case WM_SYSCOMMAND:
                    if (wParam == SC_RESTORE) pInfo = &GetInfoFromHandle(hwnd);
                    break;

                default:
                    break;
            }

            if (pInfo)
            {
                // If windowID moved or resized, update client rect

                assert(pInfo->hwnd);
                ::GetClientRect(pInfo->hwnd, pInfo->isFullScreen ? &pInfo->fullScreenArea : &pInfo->clientArea);
            }

            // Get the stored MsgProc
            MsgProc proc = (MsgProc)::GetWindowLongPtr(hwnd, 0);
            return proc ? proc(hwnd, msg, wParam, lParam) : ::DefWindowProc(hwnd, msg, wParam, lParam);
        }

        HWND GetWindowHandle(ID windowID) { return GetInfoFromID(windowID).hwnd; }

        void SetWindowCaption(ID windowID, const wchar_t* caption) { SetWindowText(GetInfoFromID(windowID).hwnd, caption); }

        Math::U32Vector4 GetWindowSize(ID windowID)
        {
            const WindowInfo& info = GetInfoFromID(windowID);
            const RECT& area = info.isFullScreen ? info.fullScreenArea : info.clientArea;
            return { (uint32)area.left, (uint32)area.top, (uint32)area.right, (uint32)area.bottom };
        }
        void ResizeWindow(const WindowInfo& info, const RECT& clientArea)
        {
            RECT windowRect = clientArea;
            ::AdjustWindowRect(&windowRect, info.style, FALSE);

            const int32 width = windowRect.right - windowRect.left;
            const int32 height = windowRect.bottom - windowRect.top;

            ::MoveWindow(info.hwnd, info.leftTop.x, info.leftTop.y, width, height, true);
        }
        void ResizeWindow(ID windowID, uint32 width, uint32 height)
        {
            WindowInfo& info = GetInfoFromID(windowID);

            if (info.style & WS_CHILD)
            {
                ::GetClientRect(info.hwnd, &info.clientArea);
            }
            else
            {
                RECT& clientArea = info.isFullScreen ? info.fullScreenArea : info.clientArea;
                clientArea.bottom = clientArea.top + height;
                clientArea.right = clientArea.left + width;

                ResizeWindow(info, clientArea);
            }
        }

        bool IsWindowFullScreen(ID windowID) { return GetInfoFromID(windowID).isFullScreen; }
        void SetWindowFullScreen(ID windowID, bool isFullScreen)
        {
            WindowInfo& info = GetInfoFromID(windowID);

            if (info.isFullScreen == isFullScreen) return;

            info.isFullScreen = isFullScreen;

            if (isFullScreen)
            {
                // record the "clientArea"
                ::GetClientRect(info.hwnd, &info.clientArea);

                RECT windowRect;
                ::GetWindowRect(info.hwnd, &windowRect);

                // record the "leftTop" position
                info.leftTop.x = windowRect.left;
                info.leftTop.y = windowRect.top;

                ::SetWindowLongPtr(info.hwnd, GWL_STYLE, 0);
                ::ShowWindow(info.hwnd, SW_MAXIMIZE);
            }
            else
            {
                ::SetWindowLongPtr(info.hwnd, GWL_STYLE, info.style);
                ResizeWindow(info, info.clientArea);
                ::ShowWindow(info.hwnd, SW_SHOWNORMAL);
            }
        }

        bool IsWindowClosed(ID windowID) { return GetInfoFromID(windowID).isClosed; }
    }

    ID Window::Create(const WindowInitDesc* const pDesc)
    {
        MsgProc callback = pDesc ? pDesc->callback : nullptr;
        HWND parent = pDesc ? pDesc->parent : nullptr;

        // Setup
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = InternalWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = callback ? sizeof(callback) : 0;
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

        WindowInfo info = {};
        info.clientArea.right = pDesc ? pDesc->width : info.clientArea.right;
        info.clientArea.bottom = pDesc ? pDesc->height : info.clientArea.bottom;
        info.style |= parent ? WS_CHILD : WS_OVERLAPPEDWINDOW;

        // Adjust the size of the windowID
        RECT windowArea = info.clientArea;
        ::AdjustWindowRect(&windowArea, info.style, FALSE);

        const int32 wndWidth = windowArea.right - windowArea.left;
        const int32 wndHeight = windowArea.bottom - windowArea.top;

        info.leftTop.x = pDesc && !pDesc->isCenter ? pDesc->left : (screenWidth - wndWidth) / 2;
        info.leftTop.y = pDesc && !pDesc->isCenter ? pDesc->top : (screenHeight - wndHeight) / 2;

        const wchar_t* caption = pDesc && pDesc->caption ? pDesc->caption : L"Xunlan Game";
        const int32 wndLeft = info.leftTop.x;
        const int32 wndTop = info.leftTop.y;

        // Create instance of the windowID
        info.hwnd = ::CreateWindowEx(
            0,                  // DWORD        dwExStyle
            wc.lpszClassName,   // LPCWSTR      lpClassName
            caption,            // LPCWSTR      lpWindowName
            info.style,         // DWORD        dwStyle
            wndLeft,            // int          X
            wndTop,             // int          Y
            wndWidth,           // int          nWidth
            wndHeight,          // int          nHeight
            parent,             // HWND         hWndParent
            NULL,               // HMENU        hMenu
            NULL,               // HINSTANCE    hInstance
            NULL                // LPVOID       lpParam
        );

        assert(info.hwnd);
        if (!info.hwnd) return INVALID_ID;

        ::SetLastError(0);

        const ID windowID = g_windows.Emplace(info);
        assert(IsValid(windowID));

        // Store windowID ID
        ::SetWindowLongPtr(info.hwnd, GWLP_USERDATA, (LONG_PTR)windowID);

        if (callback) ::SetWindowLongPtr(info.hwnd, 0, (LONG_PTR)callback);

        assert(::GetLastError() == 0);

        ::ShowWindow(info.hwnd, SW_SHOWNORMAL);
        ::UpdateWindow(info.hwnd);

        return windowID;
    }
    void Window::Remove(ID& windowID)
    {
        HWND hwnd = GetInfoFromID(windowID).hwnd;
        ::DestroyWindow(hwnd);
        g_windows.Remove(windowID);
    }

    void* Window::GetHandle(ID windowID)
    {
        assert(IsValid(windowID));
        return GetWindowHandle(windowID);
    }

    void Window::SetCaption(ID windowID, const wchar_t* caption)
    {
        assert(IsValid(windowID));
        SetWindowCaption(windowID, caption);
    }

    uint32 Window::GetWidth(ID windowID)
    {
        Math::U32Vector4 size = GetSize(windowID);
        return size.z - size.x;
    }
    uint32 Window::GetHeight(ID windowID)
    {
        Math::U32Vector4 size = GetSize(windowID);
        return size.w - size.y;
    }

    Math::U32Vector4 Window::GetSize(ID windowID)
    {
        assert(IsValid(windowID));
        return GetWindowSize(windowID);
    }
    void Window::Resize(ID windowID, uint32 width, uint32 height)
    {
        assert(IsValid(windowID));
        ResizeWindow(windowID, width, height);
    }

    bool Window::IsFullScreen(ID windowID)
    {
        assert(IsValid(windowID));
        return IsWindowFullScreen(windowID);
    }
    void Window::SetFullScreen(ID windowID, bool isFullScreen)
    {
        assert(IsValid(windowID));
        SetWindowFullScreen(windowID, isFullScreen);
    }

    bool Window::IsClosed(ID windowID)
    {
        assert(IsValid(windowID));
        return IsWindowClosed(windowID);
    }
#else
    #error "Must implement at least one platform"
#endif
}