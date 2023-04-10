#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/RasterizerState.h"

namespace Xunlan::DX12
{
    class DX12RasterizerState final : public RasterizerState
    {
    public:

        explicit DX12RasterizerState(const RasterizerStateDesc& desc) : RasterizerState(desc) {}

    public:

        D3D12_RASTERIZER_DESC GetDX12Desc() const;
    };
}