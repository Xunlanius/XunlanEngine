#include "DX12Resource.h"
#include "DX12RHI.h"

namespace Xunlan::DX12
{
    DX12DescriptorHeap::DX12DescriptorHeap(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 capacity, bool isShaderVisible)
        : m_type(type), m_descriptorSize(device.GetDescriptorHandleIncrementSize(type))
    {
        assert(capacity > 0);
        assert(capacity <= D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
        assert(m_type != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER || capacity <= D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE);

        assert(!isShaderVisible || (m_type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV && m_type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

        std::lock_guard<std::mutex> lock(m_mutex);

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
        {
            m_freeHandles[i] = i;
        }
    }

    DescriptorHandle DX12DescriptorHeap::Allocate()
    {
        std::lock_guard lock(m_mutex);

        assert(m_heap);
        assert(m_size < m_capacity);

        const uint32 freeIndex = m_freeHandles[m_size++];
        const uint32 offset = freeIndex * m_descriptorSize;

        DescriptorHandle handle = {};
        handle.index = freeIndex;
        handle.handleCPU.ptr = m_addressCPU.ptr + offset;

        return handle;
    }
    void DX12DescriptorHeap::Free(DescriptorHandle& handle)
    {
        if (!handle.IsValid()) return;

        assert(m_heap);
        assert(handle.handleCPU.ptr >= m_addressCPU.ptr);
        assert((handle.handleCPU.ptr - m_addressCPU.ptr) % m_descriptorSize == 0);

        const uint32 index = (uint32)((handle.handleCPU.ptr - m_addressCPU.ptr) / m_descriptorSize);
        assert(handle.index == index);

        DX12RHI& rhi = DX12RHI::Instance();
        const uint32 currFrameIndex = rhi.GetCurrFrameIndex();

        m_deferredFreeIndices[currFrameIndex].push_back(index);
        rhi.SetDeferredReleaseFlag(currFrameIndex);

        handle = {};
    }
    void DX12DescriptorHeap::ProcessDeferredFree(uint32 frameIndex)
    {
        std::lock_guard lock(m_mutex);

        assert(frameIndex < NUM_FRAME_BUFFERS);

        std::vector<uint32>& indices = m_deferredFreeIndices[frameIndex];
        if (indices.empty()) return;

        for (const uint32 index : indices)
        {
            m_freeHandles[--m_size] = index;
        }
        indices.clear();
    }
}