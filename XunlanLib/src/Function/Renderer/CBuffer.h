#pragma once

#include "src/Common/Common.h"
#include "RenderContext.h"
#include "src/Utility/MathTypes.h"

namespace Xunlan
{
    enum class CBufferType : uint32
    {
        PER_OBJECT,
        PER_MATERIAL,
        PER_FRAME,

        SHADOW_MAP_INDICES,
        TEXTURE_INDICES,

        COUNT,
    };

    class CBuffer
    {
    protected:

        explicit CBuffer(CBufferType type, uint32 size);

    public:

        virtual ~CBuffer() = default;

    public:

        void* GetData() const { return m_data.get(); }
        CBufferType GetType() const { return m_type; }

        virtual void Bind(const Ref<RenderContext>& context) const = 0;

    protected:

        const CBufferType m_type;

        const uint32 m_size = 0;
        std::unique_ptr<byte[]> m_data;
    };

    constexpr uint32 CBUFFER_ALIGN = 16;

    struct CBufferPerObject
    {
        Math::float4x4 m_world;
        Math::float4x4 m_invWorld;
    };
    static_assert(sizeof(CBufferPerObject) % CBUFFER_ALIGN == 0);

    constexpr uint32 MAX_NUM_DIRECTIONAL_LIGHTS = 4;
    constexpr uint32 MAX_NUM_POINT_LIGHTS = 16;
    constexpr uint32 MAX_NUM_SPOT_LIGHTS = 16;

    struct CBufferDirectionLight final
    {
        Math::float3 m_direction;
        float _0;
        Math::float3 m_color;
        float m_intensity;
        Math::float4x4 m_directionalLightViewProj;
    };

    struct CBufferPointLight final
    {
        Math::float3 m_position;
        float _0;
        Math::float3 m_color;
        float m_intensity;
    };

    struct CBufferSpotLight final
    {
        Math::float3 m_position;
        float _0;
        Math::float3 m_direction;
        float _1;
        Math::float3 m_color;
        float m_intensity;
    };

    struct CBufferPerFrame final
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
        CBufferDirectionLight m_directionLights[MAX_NUM_DIRECTIONAL_LIGHTS];

        uint32 m_numPointLights;
        Math::float3 _2;
        CBufferPointLight m_pointLights[MAX_NUM_POINT_LIGHTS];

        uint32 m_numSpotLights;
        Math::float3 _3;
        CBufferSpotLight m_spotLights[MAX_NUM_SPOT_LIGHTS];
    };

    constexpr uint32 MAX_NUM_SHADOW_MAPS = 4;

    struct CBufferShadowMapIndices final
    {
        uint32 m_shadowMapIndices[MAX_NUM_SHADOW_MAPS];
    };

    struct CBufferTextureIndices final
    {
        uint32 m_baseColorIndex;
        uint32 m_roughnessIndex;
        uint32 m_metallicIndex;
        uint32 m_normalMapIndex;
        uint32 m_heightMapIndex;
        uint32 m_AOMapIndex;
    };
}