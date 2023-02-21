#pragma once

#include "Renderer/DX12/DX12Common.h"
#include <mutex>

namespace Xunlan::Graphics::DX12
{
    struct [[nodiscard]] DescriptorHandle final
    {
        bool IsValid() const { return handleCPU.ptr != 0; }
        bool IsShaderVisible() const { return handleGPU.ptr != 0; }

        uint32 index = UINT32_MAX;
        D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = {};
        D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = {};
    };

    class DescriptorHeap final
    {
    public:

        explicit DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
        DescriptorHeap(const DescriptorHeap& rhs) = delete;
        DescriptorHeap& operator=(const DescriptorHeap& rhs) = delete;
        DescriptorHeap(DescriptorHeap&& rhs) = delete;
        DescriptorHeap& operator=(DescriptorHeap&& rhs) = delete;
        ~DescriptorHeap() = default;

    public:

        bool Initialize(uint32 capacity, bool isShaderVisible);
        void Release();

        [[nodiscard]] DescriptorHandle Allocate();
        void Free(DescriptorHandle& handle);
        void ProcessDeferredFree(uint32 frameIndex);

        D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return m_type; }
        uint32 GetDescriptorSize() const { return m_descriptorSize; }
        ID3D12DescriptorHeap* GetHeap() const { return m_heap.Get(); }
        D3D12_CPU_DESCRIPTOR_HANDLE GetAddressCPU() const { return m_addressCPU; }
        D3D12_GPU_DESCRIPTOR_HANDLE GetAddressGPU() const { return m_addressGPU; }
        uint32 GetCapacity() const { return m_capacity; }
        uint32 GetSize() const { return m_size; }
        bool IsShaderVisible() const { return m_addressGPU.ptr != 0; }

    private:

        const D3D12_DESCRIPTOR_HEAP_TYPE m_type = {};
        const uint32 m_descriptorSize = 0;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE m_addressCPU = {};
        D3D12_GPU_DESCRIPTOR_HANDLE m_addressGPU = {};

        std::unique_ptr<uint32[]> m_freeHandles = nullptr;
        std::vector<uint32> m_deferredFreeIndices[NUM_FRAME_BUFFERS] = {};

        uint32 m_capacity = 0;
        uint32 m_size = 0;

        std::mutex m_mutex = {};
    };

    struct TextureInitDesc
    {
        enum class Mode
        {
            None,
            ExistedResource,
            CreateDefault,
            CreatePlaced,
        };
        
        Mode mode = Mode::None;
        
        ID3D12Resource* resource = nullptr;

        ID3D12Heap1* heap = nullptr;
        D3D12_RESOURCE_ALLOCATION_INFO1* allocationInfo = nullptr;

        D3D12_RESOURCE_DESC* resourceDesc = nullptr;
        D3D12_RESOURCE_STATES initState = {};
        D3D12_CLEAR_VALUE clearValue = {};

        D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr;
    };

    class Texture final
    {
    public:

        Texture() = default;
        Texture(const TextureInitDesc& desc);
        Texture(const Texture& rhs) = delete;
        Texture& operator=(const Texture& rhs) = delete;
        Texture(Texture&& rhs) noexcept;
        Texture& operator=(Texture&& rhs) noexcept;
        ~Texture() { Release(); }

    public:

        void Release();

        ID3D12Resource* GetResource() const { return m_resource.Get(); }
        DescriptorHandle GetSRV() const { return m_srv; }

        static constexpr uint32 MAX_MIPS = 14;

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource = nullptr;
        DescriptorHandle m_srv = {};
    };

    class RenderTexture final
    {
    public:

        RenderTexture() = default;
        RenderTexture(const TextureInitDesc& initDesc);
        RenderTexture(const RenderTexture& rhs) = delete;
        RenderTexture& operator=(const RenderTexture& rhs) = delete;
        RenderTexture(RenderTexture&& rhs) noexcept;
        RenderTexture& operator=(RenderTexture&& rhs) noexcept;
        ~RenderTexture() { Release(); }

    public:

        void Release();

        ID3D12Resource* GetResource() const { return m_texture.GetResource(); }
        DescriptorHandle GetSRV() const { return m_texture.GetSRV(); }
        uint32 GetNumMips() const { return m_numMips; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(uint32 mipIndex) const;

    private:

        void Reset();

        Texture m_texture;
        uint32 m_numMips = 0;
        DescriptorHandle m_rtvMips[Texture::MAX_MIPS] = {};
    };

    class DepthTexture final
    {
    public:

        DepthTexture() = default;
        DepthTexture(TextureInitDesc initDesc);
        DepthTexture(const DepthTexture& rhs) = delete;
        DepthTexture& operator=(const DepthTexture& rhs) = delete;
        DepthTexture(DepthTexture&& rhs) noexcept;
        DepthTexture& operator=(DepthTexture&& rhs) noexcept;
        ~DepthTexture() { Release(); }

    public:

        void Release();

        ID3D12Resource* GetResource() const { return m_texture.GetResource(); }
        DescriptorHandle GetSRV() const { return m_texture.GetSRV(); }
        D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_dsv.handleCPU; }

    private:

        Texture m_texture;
        DescriptorHandle m_dsv = {};
    };
}