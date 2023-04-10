#include "Transformer.h"

using namespace DirectX;

namespace Xunlan
{
    TransformerComponent CreateTransformer(const TransformerInitDesc& initDesc)
    {
        TransformerComponent transformer = {};

        transformer.m_position = Math::float3(initDesc.m_position);
        transformer.m_scale = Math::float3(initDesc.m_scale);

        XMFLOAT3A rot(initDesc.m_rotation);
        XMVECTOR quat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot));

        XMFLOAT4A rotQuat;
        XMStoreFloat4A(&rotQuat, quat);
        transformer.m_rotation = rotQuat;

        return transformer;
    }

    Math::float3 GetDirection(const TransformerComponent& transformer)
    {
        const XMVECTOR rotQuat = XMLoadFloat4(&transformer.m_rotation);
        const XMVECTOR front = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        const XMVECTOR dir = XMVector3Rotate(front, rotQuat);
        XMFLOAT3 direction;
        XMStoreFloat3(&direction, dir);

        return direction;
    }
    Math::float4x4 GetWorld(const TransformerComponent& transformer)
    {
        const XMVECTOR position = XMLoadFloat3(&transformer.m_position);
        const XMVECTOR rotation = XMLoadFloat4(&transformer.m_rotation);
        const XMVECTOR scale = XMLoadFloat3(&transformer.m_scale);

        const XMMATRIX wrd = XMMatrixAffineTransformation(scale, XMQuaternionIdentity(), rotation, position);
        XMFLOAT4X4 world;
        XMStoreFloat4x4(&world, wrd);
        return world;
    }
}