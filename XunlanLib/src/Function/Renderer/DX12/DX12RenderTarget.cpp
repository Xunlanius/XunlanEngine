#include "DX12RenderTarget.h"
#include "DX12RenderContext.h"
#include "DX12RHI.h"
#include "d3dx12.h"

namespace Xunlan::DX12
{
    DX12RenderTarget::DX12RenderTarget(uint32 width, uint32 height, RenderTargetUsage usage)
        : RenderTarget(width, height, usage)
    {
        Create(width, height);
    }
    DX12RenderTarget::~DX12RenderTarget()
    {
        Clear();
    }

    Ref<DX12RenderTarget> DX12RenderTarget::Create(uint32 width, uint32 height, RenderTargetUsage usage)
    {
        return MakeRef<DX12RenderTarget>(width, height, usage);
    }

    void DX12RenderTarget::Resize(uint32 width, uint32 height)
    {
        RenderTarget::Resize(width, height);

        Clear();
        Create(width, height);
    }

    void DX12RenderTarget::Create(uint32 width, uint32 height)
    {
        switch (m_usage)
        {
        case RenderTargetUsage::DEFAULT:
        {
            CreateRenderTarget(width, height);
            CreateDepthStencil(width, height);
            break;
        }
        case RenderTargetUsage::SHADOW_MAP:
        {
            CreateDepth(width, height);
            break;
        }
        default: assert(false);
        }
    }
    void DX12RenderTarget::Clear()
    {
        switch (m_usage)
        {
        case RenderTargetUsage::DEFAULT:
        {
            ClearRenderTarget();
            ClearDepthStencil();
            break;
        }
        case RenderTargetUsage::SHADOW_MAP:
        {
            ClearDepthStencil();
            break;
        }
        default: assert(false);
        }
    }

    void DX12RenderTarget::CreateRenderTarget(uint32 width, uint32 height)
    {
        DX12RHI& rhi = DX12RHI::Instance();
        Device& device = rhi.GetDevice();

        const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        Check(device.CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                width,
                height,
                1,
                0,
                1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
            ),
            RT_INIT_STATE,
            &CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R32G32B32A32_FLOAT, clearColor),
            IID_PPV_ARGS(&m_rtTexture)
        ));

        m_rtv = rhi.GetRTVHeap().Allocate();
        m_rtSRV = rhi.GetSRVHeap().Allocate();

        device.CreateRenderTargetView(m_rtTexture.Get(), nullptr, m_rtv.handleCPU);
        device.CreateShaderResourceView(m_rtTexture.Get(), nullptr, m_rtSRV.handleCPU);

        m_currRTFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
    }
    void DX12RenderTarget::CreateDepthStencil(uint32 width, uint32 height)
    {
        DX12RHI& rhi = DX12RHI::Instance();
        Device& device = rhi.GetDevice();

        Check(device.CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(
                DXGI_FORMAT_D24_UNORM_S8_UINT,
                width,
                height,
                1,
                0,
                1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
            ),
            DS_INIT_STATE,
            &CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0),
            IID_PPV_ARGS(&m_dsTexture)
        ));

        m_dsv = rhi.GetDSVHeap().Allocate();

        device.CreateDepthStencilView(m_dsTexture.Get(), nullptr, m_dsv.handleCPU);

        m_currDSFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    }
    void DX12RenderTarget::CreateDepth(uint32 width, uint32 height)
    {
        DX12RHI& rhi = DX12RHI::Instance();
        Device& device = rhi.GetDevice();

        Check(device.CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(
                DXGI_FORMAT_R32_TYPELESS,
                width,
                height,
                1,
                0,
                1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
            ),
            DS_INIT_STATE,
            &CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
            IID_PPV_ARGS(&m_dsTexture)
        ));

        m_dsv = rhi.GetDSVHeap().Allocate();
        m_dsSRV = rhi.GetSRVHeap().Allocate();

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        device.CreateDepthStencilView(m_dsTexture.Get(), &dsvDesc, m_dsv.handleCPU);
        device.CreateShaderResourceView(m_dsTexture.Get(), &srvDesc, m_dsSRV.handleCPU);

        m_currDSFormat = DXGI_FORMAT_D32_FLOAT;
    }

    void DX12RenderTarget::ClearRenderTarget()
    {
        DX12RHI& rhi = DX12RHI::Instance();
        if (m_rtTexture) rhi.DeferredRelease(m_rtTexture);
        if (m_rtv.IsValid()) rhi.GetRTVHeap().Free(m_rtv);
        if (m_rtSRV.IsValid()) rhi.GetSRVHeap().Free(m_rtSRV);
    }
    void DX12RenderTarget::ClearDepthStencil()
    {
        DX12RHI& rhi = DX12RHI::Instance();
        if (m_dsTexture) rhi.DeferredRelease(m_dsTexture);
        if (m_dsv.IsValid()) rhi.GetDSVHeap().Free(m_dsv);
        if (m_dsSRV.IsValid()) rhi.GetSRVHeap().Free(m_dsSRV);
    }
}