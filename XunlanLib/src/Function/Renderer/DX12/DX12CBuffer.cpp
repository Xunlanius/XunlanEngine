#include "DX12CBuffer.h"
#include "DX12RenderContext.h"
#include "DX12RHI.h"
#include "Helper/d3dx12.h"
#include "src/Utility/Math/Math.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    DX12CBuffer::DX12CBuffer(size_t size)
        : CBuffer(size)
    {
        DX12RHI& rhi = (DX12RHI&)RHI::Instance();
        Device& device = rhi.GetDevice();

        const uint64 alignedSize = Math::RoundUp<D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(size);

        Check(device.CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(alignedSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_buffer))
        );

        const D3D12_RANGE range = {};
        Check(m_buffer->Map(0, &range, &m_dst));
    }
    DX12CBuffer::~DX12CBuffer()
    {
        m_buffer->Unmap(0, nullptr);
    }
}