#pragma once

#include "DX12Common.h"
#include <vector>
#include <mutex>

namespace Xunlan::DX12
{
    struct [[nodiscard]] DescriptorHandle final
    {
        bool IsValid() const { return handleCPU.ptr != 0; }

        uint32 index = UINT32_MAX;
        D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = {};
    };

    class DX12DescriptorHeap final
    {
    public:

        explicit DX12DescriptorHeap(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 capacity, bool isShaderVisible);
        DISABLE_COPY(DX12DescriptorHeap)
        DISABLE_MOVE(DX12DescriptorHeap)
        ~DX12DescriptorHeap() = default;

    public:

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

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
        D3D12_CPU_DESCRIPTOR_HANDLE m_addressCPU = {};
        D3D12_GPU_DESCRIPTOR_HANDLE m_addressGPU = {};

        std::unique_ptr<uint32[]> m_freeHandles;
        std::vector<uint32> m_deferredFreeIndices[NUM_FRAME_BUFFERS] = {};

        uint32 m_capacity = 0;
        uint32 m_size = 0;

        std::mutex m_mutex = {};
    };
}