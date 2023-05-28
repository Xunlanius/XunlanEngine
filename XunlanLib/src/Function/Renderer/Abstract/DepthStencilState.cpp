#include "DepthStencilState.h"

namespace Xunlan
{
    bool operator==(const DepthStencilState& lhs, const DepthStencilState& rhs)
    {
        return lhs.m_depthEnable == rhs.m_depthEnable &&
            lhs.m_depthWriteMask == rhs.m_depthWriteMask &&
            lhs.m_depthComparisonFunc == rhs.m_depthComparisonFunc &&
            lhs.m_stencilEnable == rhs.m_stencilEnable &&
            lhs.m_stencilReadMask == rhs.m_stencilReadMask &&
            lhs.m_stencilWriteMask == rhs.m_stencilWriteMask;
    }

    size_t DepthStencilState::Hash::operator()(const DepthStencilState& key) const
    {
        size_t result = 0;

        result += key.m_depthEnable;
        result += (size_t)key.m_depthWriteMask;
        result += (size_t)key.m_depthComparisonFunc;
        result += key.m_stencilEnable;
        result += key.m_stencilReadMask;
        result += key.m_stencilWriteMask;

        return result;
    }
}