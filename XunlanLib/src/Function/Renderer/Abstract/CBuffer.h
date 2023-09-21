#pragma once

#include "src/Common/Common.h"
#include "src/Utility/Math/MathTypes.h"

namespace Xunlan
{
    class CBuffer
    {
    protected:

        explicit CBuffer(size_t size);
        DISABLE_COPY(CBuffer)
        DISABLE_MOVE(CBuffer)
        virtual ~CBuffer() = default;

    public:

        template<typename T>
        T* GetData() const;

    protected:

        virtual void* GetData() const = 0;

    protected:

        const size_t m_size = 0;
    };

    template<typename T>
    inline T* CBuffer::GetData() const
    {
        assert(sizeof(T) <= m_size);

        return (T*)GetData();
    }

    namespace CB
    {
        struct alignas(16) PerObject final
        {
            Math::float4x4 m_world;
            Math::float4x4 m_invWorld;
        };

        struct alignas(16) PerFrame final
        {
            Math::float4x4 m_view;
            Math::float4x4 m_proj;
            Math::float4x4 m_invProj;
            Math::float4x4 m_viewProj;
            Math::float4x4 m_invViewProj;

            Math::float3 m_cameraPos;
            float m_pad0;
            Math::float3 m_cameraDir;
            float m_pad1;
        };

        constexpr uint32 MAX_NUM_POINT_LIGHTS = 16;
        constexpr uint32 MAX_NUM_SPOT_LIGHTS = 16;

        struct alignas(16) DirectionalLight final
        {
            Math::float3 m_direction;
            float m_pad0;
            Math::float3 m_color;
            float m_intensity;
            Math::float4x4 m_viewProj;
        };

        struct alignas(16) PointLight final
        {
            Math::float3 m_position;
            float m_pad0;
            Math::float3 m_color;
            float m_intensity;
        };

        struct alignas(16) SpotLight final
        {
            Math::float3 m_position;
            float m_pad0;
            Math::float3 m_direction;
            float m_pad1;
            Math::float3 m_color;
            float m_intensity;
        };

        struct alignas(16) Lights final
        {
            uint32 m_numPointLights;
            uint32 m_numSpotLights;
            Math::float2 m_pad0;

            Math::float3 m_ambientLight;
            float m_pad1;
            DirectionalLight m_directionalLight;
            PointLight m_pointLights[MAX_NUM_POINT_LIGHTS];
            SpotLight m_spotLights[MAX_NUM_SPOT_LIGHTS];
        };

        struct alignas(16) PBR final
        {
            Math::float4 m_albedo;
            float m_roughness;
            float m_metallic;
        };
    }
}