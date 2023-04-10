#pragma once

#include "src/Common/Common.h"
#include "src/Common/WindowsCommon.h"

namespace Xunlan
{
    class WindowSystem;
    class RenderSystem;

#ifdef _WIN64
    using MsgProc = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);

    struct WindowInitDesc final
    {
        const wchar_t* caption = nullptr;
        HWND parent = nullptr;
        bool isCenter = false;
        int left = 0;
        int top = 0;
        int width = 1280;
        int height = 720;
    };

    struct WindowInfo final
    {
        HWND hwnd = nullptr;
        POINT leftTop = { 0, 0 };
        uint32 width = 1280;
        uint32 height = 720;
        DWORD style = WS_VISIBLE;
    };
#else
#error Unknown platform.
#endif

    enum class Platform : uint32
    {
        DX12,
    };

    struct RenderSystemInitDesc final
    {
        Ref<WindowSystem> windowSystem;
    };

    struct RHIInitDesc final
    {
        Ref<WindowSystem> windowSystem;
    };
}