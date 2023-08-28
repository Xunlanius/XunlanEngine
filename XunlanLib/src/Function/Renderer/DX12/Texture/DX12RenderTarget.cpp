#include "DX12RenderTarget.h"
#include "../DX12RenderContext.h"
#include "../DX12RHI.h"
#include "../Helper/d3dx12.h"

namespace Xunlan::DX12
{
    DX12RenderTarget::DX12RenderTarget(uint32 width, uint32 height)
        : RenderTarget(width, height)
    {
        CreateRT(width, height);
    }

    DX12RenderTarget::~DX12RenderTarget()
    {
        ClearRT();
    }

    void DX12RenderTarget::Resize(uint32 width, uint32 height)
    {
        RenderTarget::Resize(width, height);

        ClearRT();
        CreateRT(width, height);
    }

    void DX12RenderTarget::CreateRT(uint32 width, uint32 height)
    {
        DX12RHI& rhi = DX12RHI::Instance();
        Device& device = rhi.GetDevice();

        m_format = DXGI_FORMAT_R32G32B32A32_FLOAT;

        const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        Check(device.CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(
                m_format,
                width, height,
                1, 0,
                1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
            ),
            RT_INIT_STATE,
            &CD3DX12_CLEAR_VALUE(m_format, clearColor),
            IID_PPV_ARGS(&m_resource)
        ));

        m_rtv = rhi.GetRTVHeap().Allocate();
        m_srv = rhi.GetSRVHeap().Allocate();

        device.CreateRenderTargetView(m_resource.Get(), nullptr, m_rtv.handleCPU);
        device.CreateShaderResourceView(m_resource.Get(), nullptr, m_srv.handleCPU);
    }

    void DX12RenderTarget::ClearRT()
    {
        DX12RHI& rhi = DX12RHI::Instance();
        rhi.DeferredRelease(m_resource);
        rhi.GetRTVHeap().Free(m_rtv);
        rhi.GetSRVHeap().Free(m_srv);
    }
}