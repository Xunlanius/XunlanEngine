#pragma once

#include "src/Common/Common.h"
#include "src/Function/World/ECS/ECS.h"
#include "src/Utility/Math.h"

namespace Xunlan
{
    struct TransformerInitDesc final
    {
        float m_position[3] = { 0.0f, 0.0f, 0.0f };
        float m_rotation[3] = { 0.0f, 0.0f, 0.0f };
        float m_scale[3] = { 1.0f, 1.0f, 1.0f };
    };

    struct TransformerComponent final
    {
        Math::float3 m_position;
        Math::float4 m_rotation;
        Math::float3 m_scale = { 1.0f, 1.0f, 1.0f };
    };

    TransformerComponent CreateTransformer(const TransformerInitDesc& initDesc);

    Math::float3 GetDirection(const TransformerComponent& transformer);
    Math::float4x4 GetWorld(const TransformerComponent& transformer);
}