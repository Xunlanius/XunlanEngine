#pragma once

#include "Renderer/DX12/DX12Common.h"

namespace Xunlan::Graphics::DX12
{
    struct FrameInfo
    {
        uint32 surfaceWidth = 0;
        uint32 surfaceHeight = 0;
    };
}

namespace Xunlan::Graphics::DX12::Core
{
    bool Initialize();
    void Shutdown();

    [[nodiscard]] ID CreateSurface(ID windowID);
    void RemoveSurface(ID& surfaceID);
    void RenderSurface(ID surfaceID);

    void DeferredRelease(Microsoft::WRL::ComPtr<IUnknown>& resource);
    template<typename T>
    void DeferredRelease(Microsoft::WRL::ComPtr<T>& resource)
    {
        if (!resource) return;

        Microsoft::WRL::ComPtr<IUnknown> res = nullptr;
        Check(resource.As(&res));
        DeferredRelease(res);
        resource.Reset();
    }

    // Called by descriptor heap
    void SetDeferredReleaseFlag(uint32 frameIndex);

    Device& GetDevice();
    uint32 GetCurrFrameIndex();
    DescriptorHeap& GetRTVHeap();
    DescriptorHeap& GetDSVHeap();
    DescriptorHeap& GetSRVHeap();
    DescriptorHeap& GetUAVHeap();
    DXGI_FORMAT GetRenderTargetFormat();

    uint32 GetSurfaceWidth(ID surfaceID);
    uint32 GetSurfaceHeight(ID surfaceID);
    void ResizeSurface(ID surfaceID);
}