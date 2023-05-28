#include "Vector.h"

#ifdef _WIN64

using namespace DirectX;

namespace Xunlan::Math
{
    void Normalize3(float3& vector)
    {
        XMVECTOR vec = XMLoadFloat3(&vector);
        vec = XMVector3Normalize(vec);

        XMStoreFloat3(&vector, vec);
    }

    float3 Rotate(const float3& vector, const float4& rotation)
    {
        const XMVECTOR vec = XMLoadFloat3(&vector);
        const XMVECTOR rot = XMLoadFloat4(&rotation);
        const XMVECTOR rotatedVec = XMVector3Rotate(vec, rot);

        float3 result = {};
        XMStoreFloat3(&result, rotatedVec);
        return result;
    }
}

#endif