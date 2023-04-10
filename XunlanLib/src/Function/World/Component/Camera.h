#pragma once

#include "src/Common/Common.h"
#include "src/Function/World/ECS/ECS.h"
#include "src/Utility/MathTypes.h"

namespace Xunlan
{
    enum CameraType : uint32
    {
        PERSPECTIVE,
        ORTHOGRAPHIC,
    };

    struct CameraInitDesc final
    {
        CameraType m_type;

        Math::float3 m_up;
        float m_nearZ;
        float m_farZ;
        union
        {
            float m_fov;
            float m_width;
        };
        union
        {
            float m_aspect;
            float m_height;
        };

        CameraInitDesc(CameraType type) : m_type(type), m_up(0.0, 1.0f, 0.0f), m_nearZ(0.0001f), m_farZ(10000.0f)
        {
            if (type == PERSPECTIVE)
            {
                m_fov = 0.25f;
                m_aspect = 1280.0f / 720.0f;
            }
            else
            {
                m_width = 1280.0f;
                m_height = 720.0f;
            }
        }
    };

    struct CameraComponent final
    {
        CameraType m_type;

        Math::float3 m_up;
        float m_nearZ;
        float m_farZ;
        union
        {
            float m_fov;
            float m_width;
        };
        union
        {
            float m_aspect;
            float m_height;
        };

        Math::float4x4 m_view;
        Math::float4x4 m_projection;
        Math::float4x4 m_invProjection;
        Math::float4x4 m_viewProjection;
        Math::float4x4 m_invViewProjection;

        bool m_changed;
    };

    CameraComponent AddCamera(const CameraInitDesc& initDesc);

    class CameraSystem : public ECS::ISystem
    {
    public:

        void OnUpdate();

        void SetUp(ECS::EntityID entityID, const Math::float3& up);
        void SetNearZ(ECS::EntityID entityID, float nearZ);
        void SetFarZ(ECS::EntityID entityID, float farZ);
        void SetFOV(ECS::EntityID entityID, float fov);
        void SetAspect(ECS::EntityID entityID, float aspect);
        void SetWidth(ECS::EntityID entityID, float width);
        void SetHeight(ECS::EntityID entityID, float height);

    private:

        CameraComponent& GetCamera(ECS::EntityID entityID) { return std::get<0>(m_manager.GetComponent<CameraComponent>(entityID)); }
    };
}