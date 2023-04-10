#include "Camera.h"
#include "Transformer.h"

namespace Xunlan
{
    namespace
    {
        void Update(const TransformerComponent& transformer, CameraComponent& camera)
        {
            using namespace DirectX;

            const Math::float3 direction = GetDirection(transformer);

            const XMVECTOR pos = XMLoadFloat3(&transformer.m_position);
            const XMVECTOR dir = XMLoadFloat3(&direction);
            const XMVECTOR up = XMLoadFloat3(&camera.m_up);

            const XMMATRIX view = XMMatrixLookToLH(pos, dir, up);
            XMStoreFloat4x4(&camera.m_view, view);

            if (!camera.m_changed) return;

            const XMMATRIX proj = (camera.m_type == PERSPECTIVE) ?
                XMMatrixPerspectiveFovLH(camera.m_fov * XM_PI, camera.m_aspect, camera.m_nearZ, camera.m_farZ) :
                XMMatrixOrthographicLH(camera.m_width, camera.m_height, camera.m_nearZ, camera.m_farZ);
            const XMMATRIX invProj = XMMatrixInverse(nullptr, proj);
            const XMMATRIX viewProj = view * proj;
            const XMMATRIX invViewProj = XMMatrixInverse(nullptr, viewProj);

            XMStoreFloat4x4(&camera.m_projection, proj);
            XMStoreFloat4x4(&camera.m_invProjection, invProj);
            XMStoreFloat4x4(&camera.m_viewProjection, viewProj);
            XMStoreFloat4x4(&camera.m_invViewProjection, invViewProj);

            camera.m_changed = false;
        }
    }

    CameraComponent AddCamera(const CameraInitDesc& initDesc)
    {
        CameraComponent camera = {};
        camera.m_type = initDesc.m_type;
        camera.m_up = initDesc.m_up;
        camera.m_nearZ = initDesc.m_nearZ;
        camera.m_farZ = initDesc.m_farZ;

        if (camera.m_type == PERSPECTIVE)
        {
            camera.m_fov = initDesc.m_fov;
            camera.m_aspect = initDesc.m_aspect;
        }
        else if (camera.m_type == ORTHOGRAPHIC)
        {
            camera.m_width = initDesc.m_width;
            camera.m_height = initDesc.m_height;
        }
        else assert(false && "Unknown camera type.");

        // Set the flag to true to compute the matrix
        camera.m_changed = true;

        return camera;
    }

    void CameraSystem::OnUpdate()
    {
        for (const ECS::EntityID entityID : m_entityIDs)
        {
            auto [transformer, camera] = m_manager.GetComponent<TransformerComponent, CameraComponent>(entityID);
            Update(transformer, camera);
        }
    }

    void CameraSystem::SetUp(ECS::EntityID entityID, const Math::float3& up)
    {
        CameraComponent& camera = GetCamera(entityID);
        camera.m_up = up;
    }
    void CameraSystem::SetNearZ(ECS::EntityID entityID, float nearZ)
    {
        CameraComponent& camera = GetCamera(entityID);
        camera.m_nearZ = nearZ;
        camera.m_changed = true;
    }
    void CameraSystem::SetFarZ(ECS::EntityID entityID, float farZ)
    {
        CameraComponent& camera = GetCamera(entityID);
        camera.m_farZ = farZ;
        camera.m_changed = true;
    }
    void CameraSystem::SetFOV(ECS::EntityID entityID, float fov)
    {
        CameraComponent& camera = GetCamera(entityID);
        assert(camera.m_type == PERSPECTIVE);
        camera.m_fov = fov;
        camera.m_changed = true;
    }
    void CameraSystem::SetAspect(ECS::EntityID entityID, float aspect)
    {
        CameraComponent& camera = GetCamera(entityID);
        assert(camera.m_type == PERSPECTIVE);
        camera.m_aspect = aspect;
        camera.m_changed = true;
    }
    void CameraSystem::SetWidth(ECS::EntityID entityID, float width)
    {
        CameraComponent& camera = GetCamera(entityID);
        assert(camera.m_type == ORTHOGRAPHIC);
        camera.m_width = width;
        camera.m_changed = true;
    }
    void CameraSystem::SetHeight(ECS::EntityID entityID, float height)
    {
        CameraComponent& camera = GetCamera(entityID);
        assert(camera.m_type == ORTHOGRAPHIC);
        camera.m_height = height;
        camera.m_changed = true;
    }
}