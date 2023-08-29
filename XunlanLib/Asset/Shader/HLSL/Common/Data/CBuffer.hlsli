#ifndef CBUFFER
#define CBUFFER

#include "Light.hlsli"

struct CBufferPerObject
{
    float4x4 world;
    float4x4 invWorld;
};

struct CBufferPerMaterial
{
    float4 albedo;
    float roughness;
    float metallic;
    
    uint albedoIndex;
    uint roughnessIndex;
    uint metallicIndex;
    uint normalIndex;
};

struct CBufferPerFrame
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
    
    DirectionalLight directionalLight;

    uint numPointLights;
    float3 _2;
    PointLight pointLights[MAX_NUM_POINT_LIGHTS];

    uint numSpotLights;
    float3 _3;
    SpotLight spotLights[MAX_NUM_SPOT_LIGHTS];
};

struct GBuffer
{
    uint albedoIndex;
    uint posWSIndex;
    uint normalWSIndex;
};

struct ShadowMap
{
    uint fluxIndex;
    uint posWSIndex;
    uint normalWSIndex;
    uint depthIndex;
};

struct ShadowMaps
{
    ShadowMap maps[4];
};

ConstantBuffer<CBufferPerObject> g_perObject : register(b0);
ConstantBuffer<CBufferPerMaterial> g_perMaterial : register(b1);
ConstantBuffer<CBufferPerFrame> g_perFrame : register(b2);
ConstantBuffer<GBuffer> g_GBuffer : register(b3);
ConstantBuffer<ShadowMaps> g_shadowMaps : register(b4);

#endif