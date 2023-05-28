#ifndef HELPER
#define HELPER

#include "CBuffer.hlsli"

float4 GetPos(float3 position)
{
    return mul(g_perFrame.viewProj, mul(g_perObject.world, float4(position, 1.0f)));
}
float3 GetWorldPos(float3 position)
{
    return mul(g_perObject.world, float4(position, 1.0f)).xyz;
}
float3 GetWorldNormal(float3 normal)
{
    return normalize(mul(float4(normal, 0.0f), g_perObject.invWorld).xyz);
}
float3 GetWorldTangent(float3 tangent)
{
    return normalize(mul(g_perObject.world, float4(tangent, 0.0f)).xyz);
}
float3 GetViewDir(float3 worldPos)
{
    return normalize(g_perFrame.cameraPos - worldPos);
}

#endif