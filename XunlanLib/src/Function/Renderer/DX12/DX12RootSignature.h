#pragma once

#include "DX12Common.h"

namespace Xunlan::DX12
{
    class DX12RootSignature final
    {
    public:

        DX12RootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc);
        DISABLE_COPY(DX12RootSignature)
        DISABLE_MOVE(DX12RootSignature)
        ~DX12RootSignature() { Release(); }

    public:

        Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_rootSig; }
        const D3D12_ROOT_SIGNATURE_DESC1& GetDesc() const { return m_desc; }

        uint32 GetTableMask(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const;
        uint32 GetNumDescriptorsInTable(uint32 paramIndex) const;

        void UpdateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc);
        void Release();

    private:

        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSig;
        D3D12_ROOT_SIGNATURE_DESC1 m_desc = {};

        static constexpr size_t MAX_NUM_TABLES = 32;
        uint32 m_numDescriptorsInTable[MAX_NUM_TABLES] = {};

        uint32 m_cbvSrvUavTableMask = 0;
        uint32 m_samplerTableMask = 0;
    };
}