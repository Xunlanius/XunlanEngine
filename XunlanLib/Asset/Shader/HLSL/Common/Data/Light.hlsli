#ifndef LIGHT
#define LIGHT

static const uint MAX_NUM_POINT_LIGHTS = 16;
static const uint MAX_NUM_SPOT_LIGHTS = 16;

struct DirectionalLight
{
    float3 direction;
    float3 color;
    float intensity;
    float4x4 viewProj;
};

struct PointLight
{
    float3 position;
    float3 color;
    float intensity;
};

struct SpotLight
{
    float3 position;
    float3 direction;
    float3 color;
    float intensity;
};

#endif