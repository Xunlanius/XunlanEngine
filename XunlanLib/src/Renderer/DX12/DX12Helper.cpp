#include "DX12Helper.h"
#include "Renderer/DX12/DX12Core.h"

namespace Xunlan::Graphics::DX12::Helper
{
    using namespace Microsoft::WRL;

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
        void BarrierContainer::AddTransition(ID3D12Resource& resource,
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
        void BarrierContainer::AddAliasing(ID3D12Resource& resourceBefore,
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

        void Transition(GraphicsCommandList& cmdList,
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

        void Aliasing(GraphicsCommandList& cmdList,
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

        void UAV(GraphicsCommandList& cmdList,
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