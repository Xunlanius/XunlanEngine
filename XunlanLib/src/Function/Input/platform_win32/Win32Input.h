#pragma once

#ifdef _WIN64

#include "src/Common/Common.h"
#include "src/Common/Win32Common.h"

namespace Xunlan
{
    HRESULT ProcessInputMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}

#endif