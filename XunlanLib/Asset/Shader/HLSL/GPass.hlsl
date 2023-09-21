#include "Common/Common.hlsli"

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 posWS : TEXCOORD0;
    float3 normalWS : NORMAL;
    float2 uv : TEXCOORD1;
};

struct PSOutput
{
    float4 albedo : SV_TARGET0;
    float4 posWS : SV_TARGET1;
    float4 normalWS : SV_TARGET2;
};

VSOutput VS(uint vertexIndex : SV_VertexID)
{
    const Vertex vertex = g_vertices[vertexIndex];
    
    VSOutput output;
    output.pos = GetPos(vertex.position);
    output.posWS = GetWorldPos(vertex.position);
    output.normalWS = GetWorldNormal(vertex.normal);
    output.uv = vertex.uv;
    
    return output;
}

PSOutput PS(VSOutput input)
{
    PSOutput output;
    output.albedo = g_pbr.albedo;
    output.posWS = float4(input.posWS, 1.0f);
    output.normalWS = float4(normalize(input.normalWS), 0.0f);
    
    return output;
}