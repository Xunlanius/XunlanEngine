#pragma once

#include "src/Common/Common.h"

namespace Xunlan
{
    enum class FillMode
    {
        SOLID,
        WIRE_FRAME,
    };

    enum class CullMode
    {
        NONE,
        FRONT,
        BACK,
    };

    struct RasterizerStateDesc final
    {
        FillMode m_fillMode = FillMode::SOLID;
        CullMode m_cullMode = CullMode::BACK;
        bool m_frontCounterClockWise = false;
    };

    class RasterizerState
    {
    protected:

        explicit RasterizerState(const RasterizerStateDesc& desc);
        DISABLE_COPY(RasterizerState)
        DISABLE_MOVE(RasterizerState)
        virtual ~RasterizerState() = default;

    public:

        FillMode GetFillMode() const { return m_fillMode; }
        void SetFillMode(FillMode value) { m_fillMode = value; }
        CullMode GetCullMode() const { return m_cullMode; }
        void SetCullMode(CullMode value) { m_cullMode = value; }

        friend bool operator==(const RasterizerState& lhs, const RasterizerState& rhs);

        struct Hash
        {
            size_t operator()(const RasterizerState& key) const;
        };

    protected:

        FillMode m_fillMode;
        CullMode m_cullMode;
        bool m_frontCounterClockWise;
    };
}