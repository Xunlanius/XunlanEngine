#include "DX12RootSignature.h"
#include "DX12RHI.h"
#include "Helper/d3dx12.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    DX12RootSignature::DX12RootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc)
    {
        UpdateRootSignature(desc);
    }

    uint32 DX12RootSignature::GetTableMask(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const
    {
        switch (heapType)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return m_cbvSrvUavTableMask;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: return m_samplerTableMask;
        default: assert(false); return 0;
        }
    }
    uint32 DX12RootSignature::GetNumDescriptorsInTable(uint32 paramIndex) const
    {
        assert(paramIndex < MAX_NUM_TABLES && "ParamIndex is out of range.");
        return m_numDescriptorsInTable[paramIndex];
    }

    void DX12RootSignature::UpdateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc)
    {
        Release();

        const uint32 numParams = desc.NumParameters;
        D3D12_ROOT_PARAMETER1* params = numParams > 0 ? new D3D12_ROOT_PARAMETER1[numParams] : nullptr;

        m_desc.NumParameters = numParams;
        m_desc.pParameters = params;

        for (uint32 i = 0; i < numParams; ++i)
        {
            // Copy the param
            params[i] = desc.pParameters[i];

            if (params[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            {
                const D3D12_ROOT_DESCRIPTOR_TABLE1 table = desc.pParameters[i].DescriptorTable;

                const uint32 numRanges = table.NumDescriptorRanges;
                D3D12_DESCRIPTOR_RANGE1* pRanges = numRanges > 0 ? new D3D12_DESCRIPTOR_RANGE1[numRanges] : nullptr;

                params[i].DescriptorTable = { numRanges, pRanges };

                memcpy(pRanges, table.pDescriptorRanges, sizeof(D3D12_DESCRIPTOR_RANGE1) * numRanges);

                for (uint32 j = 0; j < numRanges; ++j)
                {
                    m_numDescriptorsInTable[i] += pRanges[j].NumDescriptors;
                }

                if (numRanges > 0)
                {
                    switch (pRanges[0].RangeType)
                    {
                    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                    {
                        m_cbvSrvUavTableMask |= (1 << i);
                        break;
                    }
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                    {
                        m_samplerTableMask |= (1 << i);
                        break;
                    }
                    }
                }
            }
        }

        const uint32 numStaticSamplers = desc.NumStaticSamplers;
        D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = numStaticSamplers > 0 ? new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers] : nullptr;

        m_desc.NumStaticSamplers = numStaticSamplers;
        m_desc.pStaticSamplers = pStaticSamplers;

        if (pStaticSamplers)
        {
            memcpy(pStaticSamplers, desc.pStaticSamplers, sizeof(D3D12_STATIC_SAMPLER_DESC) * numStaticSamplers);
        }

        m_desc.Flags = desc.Flags;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionDesc = {};
        versionDesc.Init_1_1(numParams, params, numStaticSamplers, pStaticSamplers, m_desc.Flags);

        // Serialize
        ComPtr<ID3DBlob> rootSigBlob;
        ComPtr<ID3DBlob> errorBlob;
        Check(D3DX12SerializeVersionedRootSignature(
            &versionDesc,
            D3D_ROOT_SIGNATURE_VERSION_1_1,
            &rootSigBlob,
            &errorBlob)
        );

        Check(DX12RHI::Instance().GetDevice().CreateRootSignature(
            0,
            rootSigBlob->GetBufferPointer(),
            rootSigBlob->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSig))
        );
    }
    void DX12RootSignature::Release()
    {
        for (uint32 i = 0; i < m_desc.NumParameters; ++i)
        {
            const D3D12_ROOT_PARAMETER1& param = m_desc.pParameters[i];

            if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            {
                delete[] param.DescriptorTable.pDescriptorRanges;
            }
        }

        delete[] m_desc.pParameters;
        delete[] m_desc.pStaticSamplers;

        m_desc.pParameters = nullptr;
        m_desc.pStaticSamplers = nullptr;

        m_desc.NumParameters = 0;
        m_desc.NumStaticSamplers = 0;

        memset(m_numDescriptorsInTable, 0, sizeof(m_numDescriptorsInTable));

        m_samplerTableMask = 0;
        m_cbvSrvUavTableMask = 0;
    }
}