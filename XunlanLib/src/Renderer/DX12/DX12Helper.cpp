#include "DX12Helper.h"
#include "Renderer/DX12/DX12Core.h"
#include "Renderer/DX12/DX12Upload.h"

namespace Xunlan::Graphics::DX12::Helper
{
    using namespace Microsoft::WRL;

    namespace Resource
    {
        ComPtr<ID3D12Resource> CreateBuffer(
            const void* data,
            uint64 bufferSize,
            bool isCPUAccessible,
            D3D12_RESOURCE_STATES initState,
            ID3D12Heap* heap,
            uint64 heapOffset)
        {
            if (bufferSize == 0) { assert(false); return nullptr; }

            // Buffer will be used as upload buffer or CBV/UAV
            D3D12_RESOURCE_DESC desc = {};
            desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            desc.Alignment = 0;
            desc.Width = bufferSize;
            desc.Height = 1;
            desc.DepthOrArraySize = 1;
            desc.MipLevels = 1;
            desc.Format = DXGI_FORMAT_UNKNOWN;
            desc.SampleDesc = { 1, 0 };
            desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            desc.Flags = isCPUAccessible ? D3D12_RESOURCE_FLAG_NONE : D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

            D3D12_RESOURCE_STATES state = isCPUAccessible ? D3D12_RESOURCE_STATE_GENERIC_READ : initState;
            ComPtr<ID3D12Resource> resource = nullptr;
            Device& device = Core::GetDevice();

            if (heap)
            {
                Check(device.CreatePlacedResource(heap, heapOffset, &desc, state, nullptr, IID_PPV_ARGS(&resource)));
            }
            else
            {
                const D3D12_HEAP_PROPERTIES& properties = isCPUAccessible ? Heap::UPLOAD_HEAP : Heap::DEFAULT_HEAP;
                Check(device.CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &desc, state, nullptr, IID_PPV_ARGS(&resource)));
            }

            if (!data) return resource;

            if (isCPUAccessible)
            {
                D3D12_RANGE range = {};
                void* cpuAddress = nullptr;

                Check(resource->Map(0, &range, &cpuAddress));
                assert(cpuAddress);
                memcpy(cpuAddress, data, bufferSize);
                resource->Unmap(0, &range);
            }
            else // We need a intermediate upload buffer
            {
                Upload::UploadContext context(bufferSize);
                memcpy(context.GetCPUAddress(), data, bufferSize);
                context.GetCommandList()->CopyResource(resource.Get(), context.GetUploadBuffer());
                context.EndUpload();
            }

            assert(resource);
            return resource;
        }
    }

    namespace RootSig
    {
        ComPtr<ID3D12RootSignature> CreateRootSignature(Device & device, const D3D12_ROOT_SIGNATURE_DESC1 & desc)
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