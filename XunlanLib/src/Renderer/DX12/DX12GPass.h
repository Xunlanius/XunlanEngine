#pragma once

#include "Renderer/DX12/DX12Common.h"
#include "Utilities/MathTypes.h"

namespace Xunlan::Graphics::DX12
{
    struct FrameInfo;
}

namespace Xunlan::Graphics::DX12::GPass
{
    bool Initialize();
    void Shutdown();

    const RenderTexture& GetMainBuffer();
    const DepthTexture& GetDepthBuffer();

    // Called every frame
    void CheckSize(Math::UVector2 size);

    void AddTransitionForDepthPrepass(Helper::Barrier::BarrierContainer& container);
    void AddTransitionForGPass(Helper::Barrier::BarrierContainer& container);
    void AddTransitionForPostProcess(Helper::Barrier::BarrierContainer& container);

    void SetRenderTargetForDepthPrepass(GraphicsCommandList* cmdList);
    void SetRenderTargetForGPass(GraphicsCommandList* cmdList);

    void RenderDepthPrepass(GraphicsCommandList* cmdList, const FrameInfo& frameInfo);
    void Render(GraphicsCommandList* cmdList, const FrameInfo& frameInfo);
}