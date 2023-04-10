#pragma once

#include "src/Common/Common.h"

namespace Xunlan
{
    enum class DepthWriteMask : uint32
    {
        ZERO = 0,
        ALL = 1,
    };

    enum class Comparison : uint32
    {
        NEVER = 1,
        LESS = 2,
        EQUAL = 3,
        LESS_EQUAL = 4,
        GREATER = 5,
        NOT_EQUAL = 6,
        GREATER_EQUAL = 7,
        ALWAYS = 8,
    };

    class DepthStencilState
    {
    protected:

        DepthStencilState() = default;

    public:

        virtual ~DepthStencilState() = default;

    public:

        bool GetDepthEnable() const { return m_depthEnable; }
        void SetDepthEnable(bool value) { m_depthEnable = value; SetDepthWriteMask(DepthWriteMask::ZERO); }
        DepthWriteMask GetDepthWriteMask() const { return m_depthWriteMask; }
        void SetDepthWriteMask(DepthWriteMask value) { m_depthWriteMask = value; }
        Comparison GetDepthComparison() const { return m_depthComparisonFunc; }
        void SetDepthComparison(Comparison value) { m_depthComparisonFunc = value; }

        bool GetStencilEnable() const { return m_stencilEnable; }
        void SetStencilEnable(bool value) { m_stencilEnable = value; }
        unsigned char GetStencilReadMask() const { return m_stencilReadMask; }
        void SetStencilReadMask(byte value) { m_stencilReadMask = value; }
        unsigned char GetStencilWriteMask() const { return m_stencilWriteMask; }
        void SetStencilWriteMask(byte value) { m_stencilWriteMask = value; }

        friend bool operator==(const DepthStencilState& lhs, const DepthStencilState& rhs);

        struct Hash
        {
            size_t operator()(const DepthStencilState& key) const;
        };

    protected:

        bool m_depthEnable = true;
        DepthWriteMask m_depthWriteMask = DepthWriteMask::ALL;
        Comparison m_depthComparisonFunc = Comparison::LESS;

        bool m_stencilEnable = false;
        byte m_stencilReadMask = (byte)-1;
        byte m_stencilWriteMask = (byte)-1;
    };
}