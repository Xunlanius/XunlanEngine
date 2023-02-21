#pragma once

#include "Common/Common.h"

#if defined _WIN64

namespace Xunlan::Graphics
{
    using MsgProc = LRESULT (*)(HWND, UINT, WPARAM, LPARAM);

    struct WindowInitDesc
    {
        MsgProc callback = nullptr;
        const wchar_t* caption = nullptr;
        HWND parent = nullptr;
        bool isCenter = false;
        int left = 0;
        int top = 0;
        int width = 1280;
        int height = 720;
    };
}

#else

#endif