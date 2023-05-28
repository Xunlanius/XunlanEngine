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
ConstantBuffer<CBufferPerObject> g_perObject : register(b0);

struct CBufferPerMaterial
{
    
};
ConstantBuffer<CBufferPerMaterial> g_perMaterial : register(b1);

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
ConstantBuffer<CBufferPerFrame> g_perFrame : register(b2);

struct ShadowMapIndices
{
    uint ShadowMapIndices[MAX_NUM_DIRECTIONAL_LIGHTS];
};
ConstantBuffer<ShadowMapIndices> g_shadowMapIndices : register(b3);

struct TextureIndices
{
    uint BaseColorIndex;
    uint RoughnessIndex;
    uint MetallicIndex;
    uint NormalMapIndex;
    uint HeightMapIndex;
    uint AOMapIndex;
};
ConstantBuffer<TextureIndices> g_textureIndices : register(b4);

#endif