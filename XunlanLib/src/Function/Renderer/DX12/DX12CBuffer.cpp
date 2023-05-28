#include "DX12CBuffer.h"
#include "DX12RenderContext.h"
#include "DX12RHI.h"
#include "DX12RootParameter.h"
#include "Helper/d3dx12.h"
#include "src/Utility/Math/Math.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    DX12CBuffer::DX12CBuffer(CBufferType type, uint32 size) : CBuffer(type, size)
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

    void DX12CBuffer::Bind(const Ref<RenderContext>& context) const
    {
        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        memcpy(m_dst, GetData(), m_size);

        const D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_buffer->GetGPUVirtualAddress();

        switch (m_type)
        {
        case CBufferType::PER_OBJECT:
        {
            cmdList->SetGraphicsRootConstantBufferView((uint32)DefaultRootParam::PER_OBJECT, gpuAddress);
            break;
        }
        case CBufferType::PER_MATERIAL:
        {
            cmdList->SetGraphicsRootConstantBufferView((uint32)DefaultRootParam::PER_MATERIAL, gpuAddress);
            break;
        }
        case CBufferType::PER_FRAME:
        {
            cmdList->SetGraphicsRootConstantBufferView((uint32)DefaultRootParam::PER_FRAME, gpuAddress);
            break;
        }
        case CBufferType::SHADOW_MAP_INDICES:
        {
            cmdList->SetGraphicsRootConstantBufferView((uint32)DefaultRootParam::SHADOW_MAP_INDICES, gpuAddress);
            break;
        }
        case CBufferType::TEXTURE_INDICES:
        {
            cmdList->SetGraphicsRootConstantBufferView((uint32)DefaultRootParam::TEXTURE_INDICES, gpuAddress);
            break;
        }
        default: assert(false);
        }
    }
}