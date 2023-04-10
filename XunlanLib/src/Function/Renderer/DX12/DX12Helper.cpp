#include "DX12Helper.h"
#include "DX12RHI.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12::Helper
{
    namespace Resource
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> UploadToDefaultBuffer(GraphicsCommandList& cmdList, ID3D12Resource* defaultResource, const void* data, size_t byteSize)
        {
            DX12RHI& rhi = DX12RHI::Instance();
            Device& device = rhi.GetDevice();

            ComPtr<ID3D12Resource> uploadBuffer;

            Check(device.CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&uploadBuffer)));

            NAME_OBJECT_INDEX(uploadBuffer, L"Upload Buffer - Size: ", byteSize);

            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData = data;
            subresourceData.RowPitch = byteSize;
            subresourceData.SlicePitch = byteSize;

            cmdList.ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                defaultResource,
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_COPY_DEST)
            );

            UpdateSubresources<1>(&cmdList, defaultResource, uploadBuffer.Get(), 0, 0, 1, &subresourceData);

            cmdList.ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                defaultResource,
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_GENERIC_READ)
            );

            return uploadBuffer;
        }
    }

    namespace RootSig
    {
        ComPtr<ID3D12RootSignature> CreateRootSignature(Device& device, const D3D12_ROOT_SIGNATURE_DESC1& desc)
        {
            D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc = {};
            versionedDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            versionedDesc.Desc_1_1 = desc;

            ComPtr<ID3DBlob> sigBlob = nullptr;
            ComPtr<ID3DBlob> errorBlob = nullptr;
            if (FAILED(D3D12SerializeVersionedRootSignature(&versionedDesc, &sigBlob, &errorBlob)))
            {
                OutputDebugStringA(errorBlob ? (char*)errorBlob->GetBufferPointer() : "");
                return nullptr;
            }

            ComPtr<ID3D12RootSignature> signature = nullptr;
            Check(device.CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&signature)));
            assert(signature);

            return signature;
        }
    }

    namespace PSO
    {
        ComPtr<ID3D12PipelineState> CreatePipelineState(Device& device, void* stream, uint64 streamByteSize)
        {
            assert(stream);
            assert(streamByteSize);

            D3D12_PIPELINE_STATE_STREAM_DESC desc = {};
            desc.SizeInBytes = streamByteSize;
            desc.pPipelineStateSubobjectStream = stream;

            ComPtr<ID3D12PipelineState> pso = nullptr;
            Check(device.CreatePipelineState(&desc, IID_PPV_ARGS(&pso)));
            assert(pso);

            return pso;
        }
    }
    
    namespace Barrier
    {
        void BarrierContainer::AddTransition(
            ID3D12Resource& resource,
            D3D12_RESOURCE_STATES before,
            D3D12_RESOURCE_STATES after,
            D3D12_RESOURCE_BARRIER_FLAGS flags,
            uint32 subresource)
        {
            assert(m_size < MAX_NUM_BARRIERS);

            D3D12_RESOURCE_BARRIER& barrier = m_barriers[m_size++];
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = flags;
            barrier.Transition.pResource = &resource;
            barrier.Transition.Subresource = subresource;
            barrier.Transition.StateBefore = before;
            barrier.Transition.StateAfter = after;
        }
        void BarrierContainer::AddAliasing(
            ID3D12Resource& resourceBefore,
            ID3D12Resource& resourceAfter,
            D3D12_RESOURCE_BARRIER_FLAGS flags)
        {
            assert(m_size < MAX_NUM_BARRIERS);

            D3D12_RESOURCE_BARRIER& barrier = m_barriers[m_size++];
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
            barrier.Flags = flags;
            barrier.Aliasing.pResourceBefore = &resourceBefore;
            barrier.Aliasing.pResourceAfter = &resourceAfter;
        }
        void BarrierContainer::AddUAV(ID3D12Resource& resource, D3D12_RESOURCE_BARRIER_FLAGS flags)
        {
            assert(m_size < MAX_NUM_BARRIERS);

            D3D12_RESOURCE_BARRIER& barrier = m_barriers[m_size++];
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barrier.Flags = flags;
            barrier.UAV.pResource = &resource;
        }

        void BarrierContainer::Commit(GraphicsCommandList& cmdList)
        {
            if (m_size == 0) return;

            cmdList.ResourceBarrier(m_size, m_barriers);
            m_size = 0;
        }

        void Transition(
            GraphicsCommandList& cmdList,
            ID3D12Resource& resource,
            D3D12_RESOURCE_STATES before,
            D3D12_RESOURCE_STATES after,
            uint32 subresource,
            D3D12_RESOURCE_BARRIER_FLAGS flags)
        {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = flags;
            barrier.Transition.pResource = &resource;
            barrier.Transition.Subresource = subresource;
            barrier.Transition.StateBefore = before;
            barrier.Transition.StateAfter = after;

            cmdList.ResourceBarrier(1, &barrier);
        }

        void Aliasing(
            GraphicsCommandList& cmdList,
            ID3D12Resource& resourceBefore,
            ID3D12Resource& resourceAfter,
            D3D12_RESOURCE_BARRIER_FLAGS flags)
        {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
            barrier.Flags = flags;
            barrier.Aliasing.pResourceBefore = &resourceBefore;
            barrier.Aliasing.pResourceAfter = &resourceAfter;

            cmdList.ResourceBarrier(1, &barrier);
        }

        void UAV(
            GraphicsCommandList& cmdList,
            ID3D12Resource& resource,
            D3D12_RESOURCE_BARRIER_FLAGS flags)
        {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barrier.Flags = flags;
            barrier.UAV.pResource = &resource;

            cmdList.ResourceBarrier(1, &barrier);
        }
    }
}