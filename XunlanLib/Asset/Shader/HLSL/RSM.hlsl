#include "Common/Common.hlsli"

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

#define RSM_SAMPLES_COUNT 128
#define PI 3.1415926535f

struct ShadowMap
{
    uint fluxIndex;
    uint posWSIndex;
    uint normalWSIndex;
};
    
cbuffer g_rsmPass
{
    uint g_gbPosWSIndex;
    uint g_gbNormalWSIndex;
    
    ShadowMap g_shadowMap;
    
    float4 g_xi[RSM_SAMPLES_COUNT];
};

float3 ComputeVPLIrradiance(
    float3 posWS,
    float3 normalWS,
    float3 vplFlux,
    float3 vplPosWS,
    float3 vplNormalWS,
    float weight)
{
    const float3 dir = posWS - vplPosWS;
    float length2 = dot(dir, dir);
    length2 *= length2;
    return vplFlux * max(dot(vplNormalWS, dir), 0) * max(dot(normalWS, -dir), 0) / length2 * weight;
}

float3 ComputeRSM(
    float3 posWS,
    float3 normalWS,
    Texture2D fluxLSMap,
    Texture2D worldPosLSMap,
    Texture2D normalLSMap)
{
    float4 lightViewPos = mul(g_lights.directionalLight.viewProj, float4(posWS, 1.0f));
    lightViewPos.xyz /= lightViewPos.w;
    
    const float3 texSpacePos = float3(lightViewPos.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f), lightViewPos.z);
    
    float3 indirectLight = 0.0f;
    
    for (int i = 0; i < RSM_SAMPLES_COUNT; i++)
    {
        const float2 xi = g_xi[i].xy;
        const float2 vplUV = texSpacePos.xy + float2(xi.x * sin(2.0f * PI * xi.y), xi.x * cos(2.0f * PI * xi.y));
        
        const float3 vplflux = fluxLSMap.Sample(LinearClamp, vplUV).xyz;
        const float3 vplPosWS = worldPosLSMap.Sample(LinearClamp, vplUV).xyz;
        const float3 vplNormalWS = normalize(normalLSMap.Sample(LinearClamp, vplUV).xyz);
        
        indirectLight += ComputeVPLIrradiance(posWS, normalWS, vplflux, vplPosWS, vplNormalWS, xi.x * xi.x);
    }
    
    return fluxLSMap.Sample(LinearClamp, texSpacePos.xy).xyz;
}

VSOutput VS(uint vertexIndex : SV_VertexID)
{
    const Vertex vertex = g_vertices[vertexIndex];
    
    VSOutput output;
    output.pos = float4(vertex.position, 1.0f);
    output.uv = vertex.uv;
    
    return output;
}

float4 PS(VSOutput input) : SV_TARGET
{
    const Texture2D posWSMap = ResourceDescriptorHeap[g_gbPosWSIndex];
    const Texture2D normalWSMap = ResourceDescriptorHeap[g_gbNormalWSIndex];
    const Texture2D fluxLSMap = ResourceDescriptorHeap[g_shadowMap.fluxIndex];
    const Texture2D worldPosLSMap = ResourceDescriptorHeap[g_shadowMap.posWSIndex];
    const Texture2D normalLSMap = ResourceDescriptorHeap[g_shadowMap.normalWSIndex];
    
    const float3 posWS = posWSMap.Sample(LinearClamp, input.uv).xyz;
    const float3 normalWS = normalize(normalWSMap.Sample(LinearClamp, input.uv).xyz);
    
    const float3 indirectLight = ComputeRSM(posWS, normalWS, fluxLSMap, worldPosLSMap, normalLSMap);
    
    return float4(indirectLight, 1.0f);
}