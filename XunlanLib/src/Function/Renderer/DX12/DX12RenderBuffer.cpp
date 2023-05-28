#include "DX12RenderBuffer.h"
#include "DX12RHI.h"
#include "DX12Upload.h"
#include "Helper/d3dx12.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    Ref<DX12RenderBuffer> DX12RenderBuffer::Create(const RawData& rawData)
    {
        DX12RHI& rhi = (DX12RHI&)RHI::Instance();
        Device& device = rhi.GetDevice();
        UploadContext& uploadContext = rhi.GetUploadContext();

        const uint32 numElements = rawData.m_numElements;
        const uint32 stride = rawData.m_stride;
        const uint32 bufferSize = numElements * stride;

        ComPtr<ID3D12Resource> buffer;

        Check(device.CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&buffer))
        );

        uploadContext.Upload(buffer.Get(), rawData.m_buffer.get(), bufferSize);

        return MakeRef<DX12RenderBuffer>(0, numElements, stride, buffer);
    }
}