#include "Light.h"

using namespace DirectX;

namespace Xunlan
{
    Math::float4x4 GetDirectionalLightViewProj(const Math::float3& direction)
    {
        const XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&direction));

        const XMMATRIX view = XMMatrixLookToLH(XMVectorZero(), dir, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        const XMMATRIX proj = XMMatrixOrthographicLH(100, 100, 0.0001f, 10000.0f);

        Math::float4x4 viewProj = {};
        XMStoreFloat4x4(&viewProj, view* proj);

        return viewProj;
    }
}