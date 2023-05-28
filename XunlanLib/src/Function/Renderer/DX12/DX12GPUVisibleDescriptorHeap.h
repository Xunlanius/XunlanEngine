#pragma once

#include "DX12Common.h"
#include "Helper/d3dx12.h"

namespace Xunlan::DX12
{
    class DX12GPUVisibleDescriptorHeap final
    {
    public:

        explicit DX12GPUVisibleDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 capacity = 4096);

    public:

        ID3D12DescriptorHeap* GetHeap() const { return m_heap.Get(); }

        D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle);
        D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptors(const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& srcHandles);

        void Reset();

    private:

        void CreateHeap();

    private:

        const D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;
        const uint32 m_capacity;
        const uint32 m_stride;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
        uint32 m_numFreeHandles = 0;

        CD3DX12_CPU_DESCRIPTOR_HANDLE m_currCPUHandle = {};
        CD3DX12_GPU_DESCRIPTOR_HANDLE m_currGPUHandle = {};
    };
}