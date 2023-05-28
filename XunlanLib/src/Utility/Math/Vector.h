#pragma once

#include "src/Common/Common.h"
#include "MathTypes.h"

namespace Xunlan::Math
{
    inline float3 Add(const float3& lhs, const float3& rhs)
    {
        return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
    }

    inline float3 Mul(const float3& lhs, float rhs)
    {
        return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
    }
    inline float3 Mul(float lhs, const float3& rhs)
    {
        return Mul(rhs, lhs);
    }

    void Normalize3(float3& vector);
    
    float3 Rotate(const float3& vector, const float4& rotation);
}