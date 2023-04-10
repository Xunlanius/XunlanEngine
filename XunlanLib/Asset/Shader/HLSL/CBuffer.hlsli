#include "Lighting.hlsli"

static const uint MAX_NUM_DIRECTIONAL_LIGHTS = 4;
static const uint MAX_NUM_POINT_LIGHTS = 16;
static const uint MAX_NUM_SPOT_LIGHTS = 16;

cbuffer CBufferPerObject : register(b0)
{
    float4x4 world;
    float4x4 invWorld;
};

cbuffer CBufferPerMaterial : register(b1)
{
    
}

cbuffer CBufferPerFrame : register(b2)
{
    float4x4 view;
    float4x4 proj;
    float4x4 invProj;
    float4x4 viewProj;
    float4x4 invViewProj;
    
    float3 cameraPos;
    float _0;
    float3 cameraDir;
    float _1;
    
    float3 ambientLight;

    uint numDirectionalLights;
    DirectionLight directionLights[MAX_NUM_DIRECTIONAL_LIGHTS];

    uint numPointLights;
    float3 _2;
    PointLight pointLights[MAX_NUM_POINT_LIGHTS];

    uint numSpotLights;
    float3 _3;
    SpotLight spotLights[MAX_NUM_SPOT_LIGHTS];
};