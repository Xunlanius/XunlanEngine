#include "DX12DepthBuffer.h"
#include "../DX12RenderContext.h"
#include "../DX12RHI.h"
#include "../Helper/d3dx12.h"

namespace Xunlan::DX12
{
    DX12DepthBuffer::DX12DepthBuffer(uint32 width, uint32 height)
        : DepthBuffer(width, height)
    {
        CreateDepth(width, height);
    }

    DX12DepthBuffer::~DX12DepthBuffer()
    {
        ClearDepth();
    }

    void DX12DepthBuffer::Resize(uint32 width, uint32 height)
    {
        DepthBuffer::Resize(width, height);

        ClearDepth();
        CreateDepth(width, height);
    }

    void DX12DepthBuffer::CreateDepth(uint32 width, uint32 height)
    {
        DX12RHI& rhi = DX12RHI::Instance();
        Device& device = rhi.GetDevice();

        m_dxFormat = DXGI_FORMAT_D32_FLOAT;

        Check(device.CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(
                DXGI_FORMAT_R32_TYPELESS,
                width, height,
                1, 0,
                1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
            ),
            DS_INIT_STATE,
            &CD3DX12_CLEAR_VALUE(m_dxFormat, 1.0f, 0),
            IID_PPV_ARGS(&m_resource)
        ));

        m_dsv = rhi.GetDSVHeap().Allocate();
        m_srv = rhi.GetSRVHeap().Allocate();

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = m_dxFormat;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        device.CreateDepthStencilView(m_resource.Get(), &dsvDesc, m_dsv.handleCPU);
        device.CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv.handleCPU);
    }

    void DX12DepthBuffer::ClearDepth()
    {
        DX12RHI& rhi = DX12RHI::Instance();

        rhi.DeferredRelease(m_resource);
        rhi.GetDSVHeap().Free(m_dsv);
        rhi.GetSRVHeap().Free(m_srv);
    }
}