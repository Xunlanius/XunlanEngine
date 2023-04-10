#include "DX12DepthStencilState.h"

namespace Xunlan::DX12
{
    D3D12_DEPTH_STENCIL_DESC DX12DepthStencilState::GetDX12Desc() const
    {
        D3D12_DEPTH_STENCIL_DESC desc = {};

        desc.DepthEnable = m_depthEnable;
        desc.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)m_depthWriteMask;
        desc.DepthFunc = (D3D12_COMPARISON_FUNC)m_depthComparisonFunc;
        desc.StencilEnable = m_stencilEnable;
        desc.StencilReadMask = m_stencilReadMask;
        desc.StencilWriteMask = m_stencilWriteMask;

        desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
        desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

        desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
        desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

        return desc;
    }
}