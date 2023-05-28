#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/Abstract/DepthStencilState.h"

namespace Xunlan::DX12
{
    class DX12DepthStencilState final : public DepthStencilState
    {
    public:

        DX12DepthStencilState() = default;

    public:

        D3D12_DEPTH_STENCIL_DESC GetDX12Desc() const;
    };
}