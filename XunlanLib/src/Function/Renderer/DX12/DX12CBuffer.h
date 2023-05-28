#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/Abstract/CBuffer.h"

namespace Xunlan::DX12
{
    class DX12CBuffer final : public CBuffer
    {
    public:

        DX12CBuffer(CBufferType type, uint32 size);
        virtual ~DX12CBuffer() override;

    public:

        virtual void Bind(const Ref<RenderContext>& context) const override;

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
        void* m_dst = nullptr;
    };
}