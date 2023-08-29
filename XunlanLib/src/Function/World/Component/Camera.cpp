#include "Camera.h"

namespace Xunlan
{
    CameraComponent CameraSystem::AddCamera(const CameraInitDesc& initDesc)
    {
        CameraComponent camera = {};
        camera.m_type = initDesc.m_type;
        camera.m_up = initDesc.m_up;
        camera.m_nearZ = initDesc.m_nearZ;
        camera.m_farZ = initDesc.m_farZ;

        if (camera.m_type == CameraType::PERSPECTIVE)
        {
            camera.m_fov = initDesc.m_fov;
            camera.m_aspect = initDesc.m_aspect;
        }
        else if (camera.m_type == CameraType::ORTHOGRAPHIC)
        {
            camera.m_width = initDesc.m_width;
            camera.m_height = initDesc.m_height;
        }
        else assert(false && "Unknown camera type.");

        return camera;
    }

    void CameraSystem::Update()
    {
        ECS::World& world = ECS::World::Instance();
        const auto& entityIDs = world.GetView<CameraSystem>();

        for (const ECS::EntityID entityID : entityIDs)
        {
            auto [transformer, camera] = world.GetComponent<TransformerComponent, CameraComponent>(entityID);
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
    }
    void CameraSystem::SetFarZ(ECS::EntityID entityID, float farZ)
    {
        CameraComponent& camera = GetCamera(entityID);
        camera.m_farZ = farZ;
    }
    void CameraSystem::SetFOV(ECS::EntityID entityID, float fov)
    {
        CameraComponent& camera = GetCamera(entityID);
        assert(camera.m_type == CameraType::PERSPECTIVE);
        camera.m_fov = fov;
    }
    void CameraSystem::SetAspect(ECS::EntityID entityID, float aspect)
    {
        CameraComponent& camera = GetCamera(entityID);
        assert(camera.m_type == CameraType::PERSPECTIVE);
        camera.m_aspect = aspect;
    }
    void CameraSystem::SetWidth(ECS::EntityID entityID, float width)
    {
        CameraComponent& camera = GetCamera(entityID);
        assert(camera.m_type == CameraType::ORTHOGRAPHIC);
        camera.m_width = width;
    }
    void CameraSystem::SetHeight(ECS::EntityID entityID, float height)
    {
        CameraComponent& camera = GetCamera(entityID);
        assert(camera.m_type == CameraType::ORTHOGRAPHIC);
        camera.m_height = height;
    }

    void CameraSystem::Update(const TransformerComponent& transformer, CameraComponent& camera)
    {
        using namespace DirectX;

        const Math::float3 direction = TransformerSystem::GetForward(transformer);

        const XMVECTOR pos = XMLoadFloat3(&transformer.m_position);
        const XMVECTOR dir = XMLoadFloat3(&direction);
        const XMVECTOR up = XMLoadFloat3(&camera.m_up);

        const XMMATRIX view = XMMatrixLookToLH(pos, dir, up);
        const XMMATRIX proj = (camera.m_type == CameraType::PERSPECTIVE) ?
            XMMatrixPerspectiveFovLH(camera.m_fov * XM_PI, camera.m_aspect, camera.m_nearZ, camera.m_farZ) :
            XMMatrixOrthographicLH(camera.m_width, camera.m_height, camera.m_nearZ, camera.m_farZ);
        const XMMATRIX invProj = XMMatrixInverse(nullptr, proj);
        const XMMATRIX viewProj = view * proj;
        const XMMATRIX invViewProj = XMMatrixInverse(nullptr, viewProj);

        XMStoreFloat4x4(&camera.m_view, view);
        XMStoreFloat4x4(&camera.m_projection, proj);
        XMStoreFloat4x4(&camera.m_invProjection, invProj);
        XMStoreFloat4x4(&camera.m_viewProjection, viewProj);
        XMStoreFloat4x4(&camera.m_invViewProjection, invViewProj);
    }
}