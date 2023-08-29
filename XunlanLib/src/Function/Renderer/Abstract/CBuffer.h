#pragma once

#include "src/Common/Common.h"
#include "RenderContext.h"
#include "src/Utility/Math/MathTypes.h"

namespace Xunlan
{
    enum class CBufferType : uint32
    {
        PerObject,
        PerMaterial,
        PerFrame,
        GBuffer,
        ShadowMaps,

        Count,
    };

    class CBuffer
    {
    protected:

        explicit CBuffer(CBufferType type, uint32 size);
        DISABLE_COPY(CBuffer)
        DISABLE_MOVE(CBuffer)
        virtual ~CBuffer() = default;

    public:

        void* GetData() const { return m_data.get(); }
        CBufferType GetType() const { return m_type; }

        virtual void Bind(Ref<RenderContext> context) const = 0;

    protected:

        const CBufferType m_type;

        const uint32 m_size = 0;
        std::unique_ptr<byte[]> m_data;
    };

    namespace CStruct
    {
        constexpr uint32 CBUFFER_ALIGN = 16;

        struct alignas(16) PerObject final
        {
            Math::float4x4 m_world;
            Math::float4x4 m_invWorld;
        };

        struct alignas(16) PerMaterial final
        {
            Math::float4 m_albedo;
            float m_roughness;
            float m_metallic;

            uint32 m_albedoIndex;
            uint32 m_roughnessIndex;
            uint32 m_metallicIndex;
            uint32 m_normalIndex;
        };

        constexpr uint32 MAX_NUM_DIRECTIONAL_LIGHTS = 4;
        constexpr uint32 MAX_NUM_POINT_LIGHTS = 16;
        constexpr uint32 MAX_NUM_SPOT_LIGHTS = 16;

        struct alignas(16) DirectionalLight final
        {
            Math::float3 m_direction;
            float _0;
            Math::float3 m_color;
            float m_intensity;
            Math::float4x4 m_viewProj;
        };

        struct alignas(16) PointLight final
        {
            Math::float3 m_position;
            float _0;
            Math::float3 m_color;
            float m_intensity;
        };

        struct alignas(16) SpotLight final
        {
            Math::float3 m_position;
            float _0;
            Math::float3 m_direction;
            float _1;
            Math::float3 m_color;
            float m_intensity;
        };

        struct alignas(16) PerFrame final
        {
            Math::float4x4 m_view;
            Math::float4x4 m_proj;
            Math::float4x4 m_invProj;
            Math::float4x4 m_viewProj;
            Math::float4x4 m_invViewProj;

            Math::float3 m_cameraPos;
            float _0;
            Math::float3 m_cameraDir;
            float _1;

            Math::float3 m_ambientLight;

            uint32 m_numDirectionalLights;
            DirectionalLight m_directionLights[MAX_NUM_DIRECTIONAL_LIGHTS];

            uint32 m_numPointLights;
            Math::float3 _2;
            PointLight m_pointLights[MAX_NUM_POINT_LIGHTS];

            uint32 m_numSpotLights;
            Math::float3 _3;
            SpotLight m_spotLights[MAX_NUM_SPOT_LIGHTS];
        };

        struct alignas(16) GBuffer final
        {
            uint32 m_albedoIndex;
            uint32 m_positionIndex;
            uint32 m_normalIndex;
        };

        constexpr uint32 MAX_NUM_SHADOW_MAPS = 4;

        struct alignas(16) ShadowMaps final
        {
            uint32 m_shadowMapIndices[MAX_NUM_SHADOW_MAPS];
        };
    }
}