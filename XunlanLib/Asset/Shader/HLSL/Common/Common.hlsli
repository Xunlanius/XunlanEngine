#ifndef XUNLAN_COMMON
#define XUNLAN_COMMON

#include "CommonTypes.hlsli"

ConstantBuffer<PerFrameBuffer> PerFrameData : register(b0, space0);
ConstantBuffer<PerObjectBuffer> PerObjectData : register(b1, space0);
StructuredBuffer<Vertex> Vertices : register(t0, space0);

float4 GetPos(float3 position) { return mul(PerObjectData.worldViewProj, float4(position, 1.0f)); }
float3 GetWorldPos(float3 position) { return mul(PerObjectData.world, float4(position, 1.0f)).xyz; }
float3 GetWorldNormal(float3 normal) { return normalize(mul(float4(normal, 0.0f), PerObjectData.invWorld).xyz); }
float3 GetWorldTangent(float3 tangent) { return normalize(mul(PerObjectData.world, float4(tangent, 0.0f)).xyz); }

float3 GetViewDir(float3 worldPos) { return normalize(PerFrameData.cameraPos - worldPos); }

float3 ComputeDirectionLight(float3 worldPos, float3 worldNormal)
{
    float3 color = 0;
    float3 viewDir = GetViewDir(worldPos);
    
    for (uint i = 0; i < PerFrameData.numDirectionalLights; ++i)
    {
        DirectionalLight light = PerFrameData.directionLights[i];
        float3 lightDir = light.direction;
        
        float diffuse = max(0.0f, dot(worldNormal, -lightDir));
        float3 halfDir = normalize(lightDir + viewDir);
        float specular = pow(max(0.0f, dot(halfDir, worldNormal)), 16) * 0.5f;
        
        float3 lightColor = light.color;
        
        color += (diffuse + specular) * lightColor;
    }
    
    color += PerFrameData.ambientLight;
    color = saturate(color);
    return color;
}

#endif