#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/Abstract/DataBuffer.h"

namespace Xunlan::DX12
{
    class DX12DataBuffer : public DataBuffer
    {
    public:

        DX12DataBuffer(uint32 offset, uint32 numElements, uint32 stride, const Microsoft::WRL::ComPtr<ID3D12Resource>& buffer)
            : DataBuffer(offset, numElements, stride), m_buffer(buffer) {}

    public:

        static Ref<DX12DataBuffer> Create(const RawData& rawData);

        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_buffer->GetGPUVirtualAddress(); }

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
    };
}