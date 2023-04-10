#include "VertexData.hlsli"
#include "CBuffer.hlsli"
#include "TextureBuffer.hlsli"

float4 GetPos(float3 position)
{
    return mul(viewProj, mul(world, float4(position, 1.0f)));
}
float3 GetWorldPos(float3 position)
{
    return mul(world, float4(position, 1.0f)).xyz;
}
float3 GetWorldNormal(float3 normal)
{
    return normalize(mul(float4(normal, 0.0f), invWorld).xyz);
}
float3 GetWorldTangent(float3 tangent)
{
    return normalize(mul(world, float4(tangent, 0.0f)).xyz);
}