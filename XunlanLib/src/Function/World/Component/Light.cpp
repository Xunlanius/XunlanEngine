#include "Light.h"

using namespace DirectX;

namespace Xunlan
{
    Math::float4x4 GetDirectionalLightViewProj(const TransformerComponent& transformer)
    {
        const Math::float3 direction = TransformerSystem::GetForward(transformer);

        const XMVECTOR pos = XMLoadFloat3(&transformer.m_position);
        const XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&direction));

        const XMMATRIX view = XMMatrixLookToLH(pos, dir, XMVectorSet(0.f, 1.f, 0.f, 0.f));
        const XMMATRIX proj = XMMatrixOrthographicLH(128, 128, -256.f, 256.f);

        Math::float4x4 viewProj = {};
        XMStoreFloat4x4(&viewProj, view * proj);

        return viewProj;
    }
}