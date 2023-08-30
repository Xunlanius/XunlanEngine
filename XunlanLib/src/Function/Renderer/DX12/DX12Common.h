#pragma once

#include "src/Common/Common.h"
#include "src/Common/Win32Common.h"

#include <string>
#include <iostream>

#include <dxgi1_6.h>
#include <d3d12.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
//#include <DirectXCollision.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace Xunlan::DX12
{
    constexpr uint32 NUM_FRAME_BUFFERS = 3;

    using Device = ID3D12Device10;
    using Factory = IDXGIFactory7;
    using SwapChain = IDXGISwapChain4;
    using GraphicsCommandList = ID3D12GraphicsCommandList7;

    inline void Check(HRESULT hr)
    {
        if (FAILED(hr))
        {
            __debugbreak();
            abort();
        }
    }

    inline std::wstring ToWString(const char* str)
    {
        std::string s = str;
        return std::wstring(s.begin(), s.end());
    }

#ifdef _DEBUG

#define NAME_OBJECT(obj, name)                                      \
        {                                                           \
            std::wstring str = name;                                \
            (obj)->SetName(str.c_str());                            \
            /* std::wcout << L"D3D12 Object Created: " << str << '\n'; */ \
        }

#define NAME_OBJECT_INDEX(obj, name, index)                                 \
        {                                                                   \
            std::wstring str = name;                                        \
            str += L" " + std::to_wstring(index);                           \
            (obj)->SetName(str.c_str());                                    \
            /* std::wcout << L"D3D12 Object Created: " << str.c_str() << '\n'; */ \
        }

#else

#define NAME_OBJECT(obj, name)
#define NAME_OBJECT_INDEX(obj, name, index)

#endif
}