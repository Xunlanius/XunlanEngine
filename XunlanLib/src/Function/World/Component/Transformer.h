#pragma once

#include "src/Common/Common.h"
#include "src/Function/World/ECS/World.h"
#include "src/Utility/Math/Math.h"

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

    class TransformerSystem final
    {
    public:

        static TransformerComponent CreateTransformer(const TransformerInitDesc& initDesc);

        static Math::float3 GetRight(const TransformerComponent& transformer);
        static Math::float3 GetUp(const TransformerComponent& transformer);
        static Math::float3 GetForward(const TransformerComponent& transformer);

        static void RotateAxis(TransformerComponent& transformer, const Math::float3& axis, float radian);
        static void RotateGlobalX(TransformerComponent& transformer, float radian);
        static void RotateLocalX(TransformerComponent& transformer, float radian);
        static void RotateGlobalY(TransformerComponent& transformer, float radian);
        static void RotateLocalY(TransformerComponent& transformer, float radian);
        static void RotateGlobalZ(TransformerComponent& transformer, float radian);
        static void RotateLocalZ(TransformerComponent& transformer, float radian);

        static Math::float4x4 GetWorld(const TransformerComponent& transformer);
    };
}