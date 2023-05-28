#pragma once

#include "src/Common/Common.h"
#include "Transformer.h"

namespace Xunlan
{
    enum class LightType : uint32
    {
        DIRECTIONAL,
        POINT,
        SPOT,

        COUNT,
    };

    struct LightComponent final
    {
        LightType m_type;
        Math::float3 m_color;
        float m_intensity;
    };

    Math::float4x4 GetDirectionalLightViewProj(const TransformerComponent& transformer);
}