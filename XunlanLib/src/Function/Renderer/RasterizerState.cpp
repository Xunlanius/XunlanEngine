#include "RasterizerState.h"

namespace Xunlan
{
    RasterizerState::RasterizerState(const RasterizerStateDesc& desc)
        : m_fillMode(desc.m_fillMode), m_cullMode(desc.m_cullMode), m_frontCounterClockWise(desc.m_frontCounterClockWise) {}

    bool operator==(const RasterizerState& lhs, const RasterizerState& rhs)
    {
        return lhs.m_fillMode == rhs.m_fillMode &&
               lhs.m_cullMode == rhs.m_cullMode &&
               lhs.m_frontCounterClockWise == rhs.m_frontCounterClockWise;
    }

    size_t RasterizerState::Hash::operator()(const RasterizerState& key) const
    {
        size_t result = 0;
        result += (size_t)key.m_fillMode;
        result += (size_t)key.m_cullMode;
        result += key.m_frontCounterClockWise;
        return result;
    }
}