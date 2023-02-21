#include "WindowManager.h"

namespace Xunlan
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
            bool IsFullScreen = false;
            bool IsClosed = false;
        };

        WindowInfo& GetInfoFromID(ID::id_type id)
        {
            Queryer queryer(World::GetWorld());
            return queryer.GetComponent<WindowInfo>(id);
        }
        WindowInfo& GetInfoFromHandle(HWND handle)
        {
            const ID::id_type id = (ID::id_type)::GetWindowLongPtr(handle, GWLP_USERDATA);
            return GetInfoFromID(id);
        }

        // Update "WindowInfo"
        LRESULT CALLBACK InternalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            WindowInfo* pInfo = nullptr;

            switch (msg)
            {
                case WM_DESTROY:
                    GetInfoFromHandle(hwnd).IsClosed = true;
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
                // If window moved or resized, update client rect

                assert(pInfo->hwnd);
                ::GetClientRect(pInfo->hwnd, pInfo->IsFullScreen ? &pInfo->fullScreenArea : &pInfo->clientArea);
            }

            // Get the stored MsgProc
            MsgProc proc = (MsgProc)::GetWindowLongPtr(hwnd, 0);
            return proc ? proc(hwnd, msg, wParam, lParam) : ::DefWindowProc(hwnd, msg, wParam, lParam);
        }

        ID::id_type AddToWindows(WindowInfo& info)
        {
            Command cmd(World::GetWorld());
            return cmd.Create<WindowInfo>(std::forward<WindowInfo>(info));
        }
        void RemoveFromWindows(ID::id_type id)
        {
            Command cmd(World::GetWorld());
            cmd.Remove(id);
        }

        HWND GetWindowHandle(ID::id_type id) { return GetInfoFromID(id).hwnd; }

        void SetWindowCaption(ID::id_type id, const wchar_t* caption) { SetWindowText(GetInfoFromID(id).hwnd, caption); }

        DirectX::XMUINT4 GetWindowSize(ID::id_type id)
        {
            const WindowInfo& info = GetInfoFromID(id);
            const RECT& area = info.IsFullScreen ? info.fullScreenArea : info.clientArea;
            return { (uint32_t)area.left, (uint32_t)area.top, (uint32_t)area.right, (uint32_t)area.bottom };
        }
        void ResizeWindow(const WindowInfo& info, const RECT& clientArea)
        {
            RECT windowRect = clientArea;
            ::AdjustWindowRect(&windowRect, info.style, FALSE);

            const int32_t width = windowRect.right - windowRect.left;
            const int32_t height = windowRect.bottom - windowRect.top;

            ::MoveWindow(info.hwnd, info.leftTop.x, info.leftTop.y, width, height, true);
        }
        void ResizeWindow(ID::id_type id, uint32_t width, uint32_t height)
        {
            WindowInfo& info = GetInfoFromID(id);

            if (info.style & WS_CHILD)
            {
                ::GetClientRect(info.hwnd, &info.clientArea);
            }
            else
            {
                RECT& clientArea = info.IsFullScreen ? info.fullScreenArea : info.clientArea;
                clientArea.bottom = clientArea.top + height;
                clientArea.right = clientArea.left + width;

                ResizeWindow(info, clientArea);
            }
        }

        bool IsWindowFullScreen(ID::id_type id) { return GetInfoFromID(id).IsFullScreen; }
        void SetWindowFullScreen(ID::id_type id, bool isFullScreen)
        {
            WindowInfo& info = GetInfoFromID(id);

            if (info.IsFullScreen == isFullScreen) return;

            info.IsFullScreen = isFullScreen;

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

        bool IsWindowClosed(ID::id_type id) { return GetInfoFromID(id).IsClosed; }
    }

    Window WindowManager::CreateWnd(const WindowInitDesc* const pDesc)
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
        info.clientArea.right = pDesc ? info.clientArea.left + pDesc->width : info.clientArea.right;
        info.clientArea.bottom = pDesc ? info.clientArea.top + pDesc->height : info.clientArea.bottom;
        info.leftTop.x = pDesc ? pDesc->left : (screenWidth - info.clientArea.right) / 2;
        info.leftTop.y = pDesc ? pDesc->top : (screenHeight - info.clientArea.bottom) / 2;
        info.style |= parent ? WS_CHILD : WS_OVERLAPPEDWINDOW;

        // Adjust the size of the window
        RECT windowRect = info.clientArea;
        ::AdjustWindowRect(&windowRect, info.style, FALSE);

        const wchar_t* caption = pDesc && pDesc->caption ? pDesc->caption : L"Xunlan Game";
        const int32_t left = info.leftTop.x;
        const int32_t top = info.leftTop.y;
        const int32_t width = windowRect.right - windowRect.left;
        const int32_t height = windowRect.bottom - windowRect.top;

        // Create instance of the window
        info.hwnd = ::CreateWindowEx(
            /* DWORD        dwExStyle,      */  0,
            /* LPCWSTR      lpClassName,    */  wc.lpszClassName,
            /* LPCWSTR      lpWindowName,   */  caption,
            /* DWORD        dwStyle,        */  info.style,
            /* int          X,              */  left,
            /* int          Y,              */  top,
            /* int          nWidth,         */  width,
            /* int          nHeight,        */  height,
            /* HWND         hWndParent,     */  parent,
            /* HMENU        hMenu,          */  NULL,
            /* HINSTANCE    hInstance,      */  NULL,
            /* LPVOID       lpParam         */  NULL
        );

        assert(info.hwnd);
        if (!info.hwnd) return Window();

        DEBUG_OPER(::SetLastError(0));

        const ID::id_type id = AddToWindows(info);
        assert(ID::IsValid(id));

        // Store window ID
        ::SetWindowLongPtr(info.hwnd, GWLP_USERDATA, (LONG_PTR)id);

        if (callback) ::SetWindowLongPtr(info.hwnd, 0, (LONG_PTR)callback);

        assert(::GetLastError() == 0);

        ::ShowWindow(info.hwnd, SW_SHOWNORMAL);
        ::UpdateWindow(info.hwnd);

        return Window(id);
    }
    void WindowManager::RemoveWnd(Window window)
    {
        ID::id_type id = window.GetID();
        WindowInfo& info = GetInfoFromID(id);
        ::DestroyWindow(info.hwnd);
        RemoveFromWindows(id);
    }

    void* WindowManager::GetHandle(Window window)
    {
        assert(window.IsValid());
        return GetWindowHandle(window);
    }

    void WindowManager::SetCaption(Window window, const wchar_t* caption)
    {
        assert(window.IsValid());
        SetWindowCaption(window, caption);
    }

    uint32_t WindowManager::GetWidth(Window window)
    {
        DirectX::XMUINT4 size = GetSize(window);
        return size.z - size.x;
    }
    uint32_t WindowManager::GetHeight(Window window)
    {
        DirectX::XMUINT4 size = GetSize(window);
        return size.w - size.y;
    }

    DirectX::XMUINT4 WindowManager::GetSize(Window window)
    {
        assert(window.IsValid());
        return GetWindowSize(window);
    }
    void WindowManager::Resize(Window window, uint32_t width, uint32_t height)
    {
        assert(window.IsValid());
        ResizeWindow(window, width, height);
    }

    bool WindowManager::IsFullScreen(Window window)
    {
        assert(window.IsValid());
        return IsWindowFullScreen(window);
    }
    void WindowManager::SetFullScreen(Window window, bool isFullScreen)
    {
        assert(window.IsValid());
        SetWindowFullScreen(window, isFullScreen);
    }

    bool WindowManager::IsClosed(Window window)
    {
        assert(window.IsValid());
        return IsWindowClosed(window);
    }

#else
    #error "Must implement at least one platform"
#endif
}