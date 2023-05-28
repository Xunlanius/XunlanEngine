#include "Transformer.h"

using namespace DirectX;

namespace Xunlan
{
    TransformerComponent TransformerSystem::CreateTransformer(const TransformerInitDesc& initDesc)
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

    Math::float3 TransformerSystem::GetRight(const TransformerComponent& transformer)
    {
        const XMVECTOR rot = XMLoadFloat4(&transformer.m_rotation);
        const XMVECTOR localRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

        XMFLOAT3 right;
        XMStoreFloat3(&right, XMVector3Rotate(localRight, rot));

        return right;
    }
    Math::float3 TransformerSystem::GetUp(const TransformerComponent& transformer)
    {
        const XMVECTOR rot = XMLoadFloat4(&transformer.m_rotation);
        const XMVECTOR localUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        XMFLOAT3 up;
        XMStoreFloat3(&up, XMVector3Rotate(localUp, rot));

        return up;
    }
    Math::float3 TransformerSystem::GetForward(const TransformerComponent& transformer)
    {
        const XMVECTOR rot = XMLoadFloat4(&transformer.m_rotation);
        const XMVECTOR localForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        XMFLOAT3 forward;
        XMStoreFloat3(&forward, XMVector3Rotate(localForward, rot));

        return forward;
    }

    void TransformerSystem::RotateAxis(TransformerComponent& transformer, const Math::float3& axis, float radian)
    {
        const XMVECTOR oldRot = XMLoadFloat4(&transformer.m_rotation);
        const XMVECTOR newRot = XMQuaternionRotationAxis(XMLoadFloat3(&axis), radian);

        XMStoreFloat4(&transformer.m_rotation, XMQuaternionMultiply(oldRot, newRot));
    }
    void TransformerSystem::RotateGlobalX(TransformerComponent& transformer, float radian)
    {
        RotateAxis(transformer, { 1.0f, 0.0f, 0.0f }, radian);
    }
    void TransformerSystem::RotateLocalX(TransformerComponent& transformer, float radian)
    {
        RotateAxis(transformer, GetRight(transformer), radian);
    }
    void TransformerSystem::RotateGlobalY(TransformerComponent& transformer, float radian)
    {
        RotateAxis(transformer, { 0.0f, 1.0f, 0.0f }, radian);
    }
    void TransformerSystem::RotateLocalY(TransformerComponent& transformer, float radian)
    {
        RotateAxis(transformer, GetUp(transformer), radian);
    }
    void TransformerSystem::RotateGlobalZ(TransformerComponent& transformer, float radian)
    {
        RotateAxis(transformer, { 0.0f, 0.0f, 1.0f }, radian);
    }
    void TransformerSystem::RotateLocalZ(TransformerComponent& transformer, float radian)
    {
        RotateAxis(transformer, GetForward(transformer), radian);
    }

    Math::float4x4 TransformerSystem::GetWorld(const TransformerComponent& transformer)
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