#include "DX12GPUVisibleDescriptorHeap.h"
#include "DX12RHI.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    DX12GPUVisibleDescriptorHeap::DX12GPUVisibleDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 capacity)
        : m_heapType(heapType),
        m_capacity(capacity),
        m_stride(DX12RHI::Instance().GetDevice().GetDescriptorHandleIncrementSize(heapType))
    {
        CreateHeap();
        Reset();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DX12GPUVisibleDescriptorHeap::CopyDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle)
    {
        return CopyDescriptors({ srcHandle });
    }
    D3D12_GPU_DESCRIPTOR_HANDLE DX12GPUVisibleDescriptorHeap::CopyDescriptors(const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& srcHandles)
    {
        const uint32 numHandles = (uint32)srcHandles.size();
        assert(numHandles <= m_numFreeHandles);

        Device& device = DX12RHI::Instance().GetDevice();

        for (const auto cpuHandle : srcHandles)
        {
            if (cpuHandle.ptr > 0) device.CopyDescriptorsSimple(1, m_currCPUHandle, cpuHandle, m_heapType);
            else
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
                desc.Format = DXGI_FORMAT_R32_FLOAT;
                desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                device.CreateShaderResourceView(nullptr, &desc, m_currCPUHandle);
            }

            m_currCPUHandle.Offset(1, m_stride);
            m_currGPUHandle.Offset(1, m_stride);
        }

        return m_currGPUHandle;
    }

    void DX12GPUVisibleDescriptorHeap::Reset()
    {
        m_numFreeHandles = m_capacity;
        m_currCPUHandle = m_heap->GetCPUDescriptorHandleForHeapStart();
        m_currGPUHandle = m_heap->GetGPUDescriptorHandleForHeapStart();
    }

    void DX12GPUVisibleDescriptorHeap::CreateHeap()
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = m_heapType;
        desc.NumDescriptors = (uint32_t)m_capacity;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;

        Check(DX12RHI::Instance().GetDevice().CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));
    }
}