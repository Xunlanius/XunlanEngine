#ifndef CBUFFER
#define CBUFFER

#include "Light.hlsli"

struct PBR
{
    float4 albedo;
    float roughness;
    float metallic;
};

struct PerObject
{
    float4x4 world;
    float4x4 invWorld;
};

struct PerFrame
{
    float4x4 view;
    float4x4 proj;
    float4x4 invProj;
    float4x4 viewProj;
    float4x4 invViewProj;
    
    float3 cameraPos;
    float3 cameraDir;
};

struct Lights
{
    uint numPointLights;
    uint numSpotLights;
    
    float3 ambientLight;

    DirectionalLight directionalLight;
    PointLight pointLights[MAX_NUM_POINT_LIGHTS];
    SpotLight spotLights[MAX_NUM_SPOT_LIGHTS];
};

// register b0, b1, b2 is reserved for shader

ConstantBuffer<PBR> g_pbr : register(b3);
ConstantBuffer<PerObject> g_perObject : register(b4);
ConstantBuffer<PerFrame> g_perFrame : register(b5);
ConstantBuffer<Lights> g_lights : register(b6);

#endif