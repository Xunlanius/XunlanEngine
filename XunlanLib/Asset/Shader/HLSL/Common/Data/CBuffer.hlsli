#ifndef CBUFFER
#define CBUFFER

static const uint MAX_NUM_DIRECTIONAL_LIGHTS = 4;
static const uint MAX_NUM_POINT_LIGHTS = 16;
static const uint MAX_NUM_SPOT_LIGHTS = 16;

struct DirectionalLight
{
    float3 direction;
    float _0;
    float3 color;
    float intensity;
    float4x4 viewProj;
};

struct PointLight
{
    float3 position;
    float _0;
    float3 color;
    float intensity;
};

struct SpotLight
{
    float3 position;
    float _0;
    float3 direction;
    float _1;
    float3 color;
    float intensity;
};

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
    float2 pad0;
    
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

    uint numDirectionalLights;
    DirectionalLight directionLights[MAX_NUM_DIRECTIONAL_LIGHTS];

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
    uint positionIndex;
    uint normalIndex;
};

struct ShadowMaps
{
    uint shadowMapIndices[MAX_NUM_DIRECTIONAL_LIGHTS];
};

ConstantBuffer<CBufferPerObject> g_perObject : register(b0);
ConstantBuffer<CBufferPerMaterial> g_perMaterial : register(b1);
ConstantBuffer<CBufferPerFrame> g_perFrame : register(b2);
ConstantBuffer<GBuffer> g_GBuffer : register(b3);
ConstantBuffer<ShadowMaps> g_shadowMaps : register(b4);

#endif