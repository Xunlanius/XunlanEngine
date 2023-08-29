#include "DX12RasterizerState.h"

namespace Xunlan::DX12
{
    D3D12_RASTERIZER_DESC DX12RasterizerState::GetDX12Desc() const
    {
        D3D12_RASTERIZER_DESC desc = {};

        switch (m_fillMode)
        {
        case FillMode::SOLID: desc.FillMode = D3D12_FILL_MODE_SOLID;
            break;
        case FillMode::WIRE_FRAME: desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
            break;
        default: assert(false);
        }

        switch (m_cullMode)
        {
        case CullMode::NONE: desc.CullMode = D3D12_CULL_MODE_NONE;
            break;
        case CullMode::FRONT: desc.CullMode = D3D12_CULL_MODE_FRONT;
            break;
        case CullMode::BACK: desc.CullMode = D3D12_CULL_MODE_BACK;
            break;
        default: assert(false);
        }

        desc.FrontCounterClockwise = m_frontCounterClockWise;
        desc.DepthBias = 0;
        desc.DepthBiasClamp = 0.0f;
        desc.SlopeScaledDepthBias = 0.0f;
        desc.DepthClipEnable = m_depthClipEnable;
        desc.MultisampleEnable = TRUE;
        desc.AntialiasedLineEnable = FALSE;
        desc.ForcedSampleCount = 0;
        desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        return desc;
    }
}