#include "RasterizerState.h"

namespace Xunlan
{
    RasterizerState::RasterizerState(const RasterizerStateDesc& desc)
        : m_fillMode(desc.m_fillMode),
        m_cullMode(desc.m_cullMode),
        m_frontCounterClockWise(desc.m_frontCounterClockWise),
        m_depthClipEnable(desc.m_depthClipEnable) {}

    bool operator==(const RasterizerState& lhs, const RasterizerState& rhs)
    {
        return lhs.m_fillMode == rhs.m_fillMode &&
               lhs.m_cullMode == rhs.m_cullMode &&
               lhs.m_frontCounterClockWise == rhs.m_frontCounterClockWise &&
               lhs.m_depthClipEnable == rhs.m_depthClipEnable;
    }

    size_t RasterizerState::Hash::operator()(const RasterizerState& key) const
    {
        return (size_t)key.m_fillMode +
               (size_t)key.m_cullMode +
               key.m_frontCounterClockWise +
               key.m_depthClipEnable;
    }
}