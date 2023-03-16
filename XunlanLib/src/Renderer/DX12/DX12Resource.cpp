#include "DX12Resource.h"
#include "Renderer/DX12/DX12Core.h"

namespace Xunlan::Graphics::DX12
{
    /*__________________________ Descriptor Heap __________________________*/

    DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
        : m_type(type), m_descriptorSize(Core::GetDevice().GetDescriptorHandleIncrementSize(type)) {}

    bool DescriptorHeap::Initialize(uint32 capacity, bool isShaderVisible)
    {
        assert(capacity > 0);
        assert(capacity <= D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
        assert(m_type != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER || capacity <= D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE);

        assert(!isShaderVisible || (m_type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV && m_type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_heap) Release();
        Device& device = Core::GetDevice();

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = m_type;
        desc.NumDescriptors = capacity;
        desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;

        Check(device.CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));
        m_addressCPU = m_heap->GetCPUDescriptorHandleForHeapStart();
        m_addressGPU = isShaderVisible ? m_heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE();
        m_freeHandles = std::make_unique<uint32[]>(capacity);
        m_capacity = capacity;

        for (uint32 i = 0; i < capacity; ++i)
            m_freeHandles[i] = i;
        for (uint32 i = 0; i < NUM_FRAME_BUFFERS; ++i)
            m_deferredFreeIndices[i].clear();

        return true;
    }
    void DescriptorHeap::Release()
    {
        if (!m_heap) return;
        assert(m_size == 0);

        Core::DeferredRelease<ID3D12DescriptorHeap>(m_heap);
    }

    DescriptorHandle DescriptorHeap::Allocate()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        assert(m_heap);
        assert(m_size < m_capacity);

        const uint32 freeIndex = m_freeHandles[m_size++];
        const uint32 offset = freeIndex * m_descriptorSize;

        DescriptorHandle handle = {};
        handle.index = freeIndex;
        handle.handleCPU.ptr = m_addressCPU.ptr + offset;
        if (IsShaderVisible())
        {
            handle.handleGPU.ptr = m_addressGPU.ptr + offset;
        }

        return handle;
    }
    void DescriptorHeap::Free(DescriptorHandle& handle)
    {
        if (!handle.IsValid()) return;

        assert(m_heap);
        assert(handle.handleCPU.ptr >= m_addressCPU.ptr);
        assert((handle.handleCPU.ptr - m_addressCPU.ptr) % m_descriptorSize == 0);

        const uint32 index = (uint32)((handle.handleCPU.ptr - m_addressCPU.ptr) / m_descriptorSize);
        assert(handle.index == index);

        m_deferredFreeIndices[Core::GetCurrFrameIndex()].push_back(index);
        Core::SetDeferredReleaseFlag(Core::GetCurrFrameIndex());

        handle = {};
    }
    void DescriptorHeap::ProcessDeferredFree(uint32 frameIndex)
    {
        assert(frameIndex < NUM_FRAME_BUFFERS);

        std::vector<uint32>& indices = m_deferredFreeIndices[frameIndex];
        if (indices.empty()) return;

        std::lock_guard lock(m_mutex);
        if (indices.empty()) return;

        for (const uint32 index : indices)
        {
            m_freeHandles[--m_size] = index;
        }
        indices.clear();
    }

    /*__________________________ Texture __________________________*/

    Texture::Texture(const TextureInitDesc& initDesc)
    {
        Device& device = Core::GetDevice();

        assert(initDesc.mode != TextureInitDesc::Mode::None);
        
        const D3D12_RESOURCE_DESC* resourceDesc = initDesc.resourceDesc;
        const D3D12_CLEAR_VALUE* clearValue =
            resourceDesc &&
            ((resourceDesc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) ||
            (resourceDesc->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            ? &initDesc.clearValue : nullptr;

        switch (initDesc.mode)
        {
        case TextureInitDesc::Mode::ExistedResource:
            assert(initDesc.resource);
            m_resource = initDesc.resource;
            break;

        case TextureInitDesc::Mode::CreatePlaced:
            assert(initDesc.heap);
            assert(initDesc.allocationInfo);
            assert(initDesc.resourceDesc);

            Check(device.CreatePlacedResource(
                initDesc.heap,
                initDesc.allocationInfo->Offset,
                resourceDesc,
                initDesc.initState,
                clearValue,
                IID_PPV_ARGS(&m_resource)));
            break;

        case TextureInitDesc::Mode::CreateDefault:
            assert(initDesc.resourceDesc);

            Check(device.CreateCommittedResource(
                &Helper::Heap::DEFAULT_HEAP,
                D3D12_HEAP_FLAG_NONE,
                resourceDesc,
                initDesc.initState,
                clearValue,
                IID_PPV_ARGS(&m_resource)));
            break;

        default:
            assert(false);
        }

        assert(m_resource);
        m_srv = Core::GetSRVHeap().Allocate();
        device.CreateShaderResourceView(m_resource.Get(), initDesc.srvDesc, m_srv.handleCPU);
    }
    Texture::Texture(Texture&& rhs) noexcept
        : m_resource(std::move(rhs.m_resource)), m_srv(rhs.m_srv)
    {
        rhs.m_srv = {};
    }
    Texture& Texture::operator=(Texture&& rhs) noexcept
    {
        if (this == &rhs) return *this;

        Release();

        m_resource = std::move(rhs.m_resource);
        m_srv = rhs.m_srv;

        rhs.m_srv = {};

        return *this;
    }

    void Texture::Release()
    {
        Core::GetSRVHeap().Free(m_srv);
        Core::DeferredRelease(m_resource);
    }

    /*__________________________ Render Texture __________________________*/

    RenderTexture::RenderTexture(const TextureInitDesc& initDesc) : m_texture(initDesc)
    {
        // Get actual resource desc
        const D3D12_RESOURCE_DESC resourceDesc = m_texture.GetResource()->GetDesc();

        m_numMips = resourceDesc.MipLevels;
        assert(m_numMips > 0 && m_numMips <= Texture::MAX_MIPS);

        DescriptorHeap& rtvHeap = Core::GetRTVHeap();

        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format = resourceDesc.Format;
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;

        Device& device = Core::GetDevice();

        for (uint32 i = 0; i < m_numMips; ++i)
        {
            m_rtvMips[i] = rtvHeap.Allocate();
            device.CreateRenderTargetView(m_texture.GetResource(), &desc, m_rtvMips[i].handleCPU);
            ++desc.Texture2D.MipSlice;
        }
    }
    RenderTexture::RenderTexture(RenderTexture&& rhs) noexcept
        : m_texture(std::move(rhs.m_texture)), m_numMips(rhs.m_numMips)
    {
        std::memcpy(m_rtvMips, rhs.m_rtvMips, m_numMips * sizeof(DescriptorHandle));
        rhs.Reset();
    }
    RenderTexture& RenderTexture::operator=(RenderTexture&& rhs) noexcept
    {
        if (this == &rhs) return *this;
        Release();

        m_texture = std::move(rhs.m_texture);
        m_numMips = rhs.m_numMips;
        std::memcpy(m_rtvMips, rhs.m_rtvMips, m_numMips * sizeof(DescriptorHandle));

        rhs.Reset();
        return *this;
    }

    void RenderTexture::Release()
    {
        for (uint32 i = 0; i < m_numMips; ++i) Core::GetRTVHeap().Free(m_rtvMips[i]);
        m_texture.Release();
        m_numMips = 0;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderTexture::GetRTV(uint32 mipIndex) const
    {
        assert(mipIndex < m_numMips);
        return m_rtvMips[mipIndex].handleCPU;
    }

    void RenderTexture::Reset()
    {
        m_numMips = 0;
        for (uint32 i = 0; i < m_numMips; ++i) m_rtvMips[i] = {};
    }
    
    /*__________________________ Depth Texture __________________________*/

    DepthTexture::DepthTexture(TextureInitDesc initDesc)
    {
        assert(initDesc.resourceDesc);

        const DXGI_FORMAT dsvFormat = initDesc.resourceDesc->Format;

        // DXGI_FORMAT_D32_FLOAT: DSV format
        // DXGI_FORMAT_R32_FLOAT: SRV format

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        if (dsvFormat == DXGI_FORMAT_D32_FLOAT)
        {
            // The format of the resource should compatible for Read/Write
            initDesc.resourceDesc->Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        }
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        assert(!initDesc.srvDesc && !initDesc.resource);
        initDesc.srvDesc = &srvDesc;

        m_texture = Texture(initDesc);

        // Create DSV
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = dsvFormat;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
            dsvDesc.Texture2D.MipSlice = 0;

            m_dsv = Core::GetDSVHeap().Allocate();
            Core::GetDevice().CreateDepthStencilView(m_texture.GetResource(), &dsvDesc, m_dsv.handleCPU);
        }
    }
    DepthTexture::DepthTexture(DepthTexture&& rhs) noexcept
        : m_texture(std::move(rhs.m_texture)), m_dsv(rhs.m_dsv)
    {
        rhs.m_dsv = {};
    }
    DepthTexture& DepthTexture::operator=(DepthTexture&& rhs) noexcept
    {
        if (this == &rhs) return *this;

        m_texture = std::move(rhs.m_texture);
        m_dsv = rhs.m_dsv;

        rhs.m_dsv = {};
        return *this;
    }

    void DepthTexture::Release()
    {
        Core::GetDSVHeap().Free(m_dsv);
        m_texture.Release();
    }
}