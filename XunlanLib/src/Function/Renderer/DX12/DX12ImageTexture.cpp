#include "DX12ImageTexture.h"
#include "DX12RHI.h"
#include "DX12Upload.h"
#include "Helper/d3dx12.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    DX12ImageTexture::~DX12ImageTexture()
    {
        DX12RHI& rhi = DX12RHI::Instance();
        rhi.GetSRVHeap().Free(m_handle);
        rhi.DeferredRelease(m_texture);
    }

    Ref<ImageTexture> DX12ImageTexture::Create(const CRef<RawTexture>& rawTexture)
    {
        DX12RHI& rhi = DX12RHI::Instance();
        Device& device = rhi.GetDevice();
        UploadContext& uploadContext = rhi.GetUploadContext();

        const uint32 width = rawTexture->GetWidth();
        const uint32 height = rawTexture->GetHeight();
        const uint32 bufferSize = width * height * rawTexture->GetPixelSize();

        ComPtr<ID3D12Resource> texture;

        Check(device.CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height),
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&texture)));

        uploadContext.Upload(texture.Get(), rawTexture->GetData(), bufferSize);

        DescriptorHandle handle = rhi.GetSRVHeap().Allocate();
        device.CreateShaderResourceView(texture.Get(), nullptr, handle.handleCPU);

        return MakeRef<DX12ImageTexture>(rawTexture, texture, handle);
    }
}