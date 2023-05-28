#include "Light.h"

using namespace DirectX;

namespace Xunlan
{
    Math::float4x4 GetDirectionalLightViewProj(const TransformerComponent& transformer)
    {
        const Math::float3 direction = TransformerSystem::GetForward(transformer);

        const XMVECTOR pos = XMLoadFloat3(&transformer.m_position);
        const XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&direction));

        const XMMATRIX view = XMMatrixLookToLH(pos, dir, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        const XMMATRIX proj = XMMatrixOrthographicLH(128, 128, 1.0f, 500.0f);

        Math::float4x4 viewProj = {};
        XMStoreFloat4x4(&viewProj, view * proj);

        return viewProj;
    }
}