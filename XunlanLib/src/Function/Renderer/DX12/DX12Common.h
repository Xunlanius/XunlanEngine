#pragma once

#include "src/Common/Common.h"
#include "src/Common/Win32Common.h"

#include <string>

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
            (obj)->SetName(name);                                   \
            OutputDebugString(L"________D3D12 Object Created: ");   \
            OutputDebugString(name);                                \
            OutputDebugString(L"\n");                               \
        }

#define NAME_OBJECT_INDEX(obj, name, index)                         \
        {                                                           \
            std::wstring indexName = name;                          \
            indexName += L" " + std::to_wstring(index);             \
            (obj)->SetName(indexName.c_str());                      \
            OutputDebugString(L"________D3D12 Object Created: ");   \
            OutputDebugString(indexName.c_str());                   \
            OutputDebugString(L"\n");                               \
        }

#else

#define NAME_OBJECT(obj, name)
#define NAME_OBJECT_INDEX(obj, name, index)

#endif
}