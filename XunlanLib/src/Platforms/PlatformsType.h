#pragma once

#include "Common/Common.h"

namespace Xunlan::Graphics
{
#ifdef _WIN64
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
#else

#endif
}