#include "RenderPassManager.h"
#include "RHI.h"
#include "DX12/DX12RHI.h"
#include "DX12/DX12Surface.h"
#include "DX12/DX12Helper.h"
#include "Pass/GPass.h"
#include "Pass/PostProcessPass.h"

namespace Xunlan
{
    void RenderPassManager::Initialize()
    {
        m_gPass = std::make_unique<DX12::GPass>();
        m_postProcessPass = std::make_unique<DX12::PostProcessPass>();

        RenderPassInitDesc initDesc = {};

        m_gPass->Initialize(initDesc);
        m_postProcessPass->Initialize(initDesc);
    }
    void RenderPassManager::Shutdown()
    {
        RHI& rhi = RHI::Instance();

        rhi.Flush();

        m_gPass->Shutdown();
        m_postProcessPass->Shutdown();

        m_gPass.reset();
        m_postProcessPass.reset();

        rhi.Shutdown();
    }

    void RenderPassManager::RenderForwardPlus(const RenderResource& renderResource)
    {
        DX12::DX12RHI* dx12RHI = (DX12::DX12RHI*)(&RHI::Instance());
        DX12::GPass* gPass = (DX12::GPass*)m_gPass.get();
        DX12::PostProcessPass* postProcessPass = (DX12::PostProcessPass*)m_postProcessPass.get();

        dx12RHI->BeginRecord();

        // Prepare for the frame info
        const uint32 frameIndex = dx12RHI->GetCurrFrameIndex();
        dx12RHI->ProcessDeferredRelease(frameIndex);

        DX12::DX12CBuffer& cBuffer = dx12RHI->GetCBuffer();
        cBuffer.Reset();

        const DX12::DX12FrameInfo frameInfo = dx12RHI->GetFrameInfo(renderResource, cBuffer, frameIndex, 16.7f);

        gPass->CheckSize({ frameInfo.surfaceWidth, frameInfo.surfaceHeight });

        DX12::GraphicsCommandList& cmdList = dx12RHI->GetCmdList();
        DX12::DX12Surface& surface = dx12RHI->GetSurface();
        DX12::Helper::Barrier::BarrierContainer& container = dx12RHI->GetBarrierContainer();

        cmdList.RSSetViewports(1, &surface.GetViewport());
        cmdList.RSSetScissorRects(1, &surface.GetScissorRect());

        ID3D12DescriptorHeap* const heaps[] = { dx12RHI->GetSRVHeap().GetHeap() };
        cmdList.SetDescriptorHeaps(_countof(heaps), heaps);

        gPass->PrepareForDepth();
        gPass->RenderDepth(frameInfo);

        gPass->PrepareForLight(container);
        gPass->RenderLight(frameInfo);

        dx12RHI->BeforeRenderToScreen();

        postProcessPass->Prepare(surface.GetRTV(), gPass->GetMainBufferPosition());
        postProcessPass->Render();

        dx12RHI->BeforePresent();

        dx12RHI->EndRecord();

        dx12RHI->Present();
    }
}