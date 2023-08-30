#include "Common/Common.hlsli"

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 worldNormal : NORMAL;
    float2 uv : TEXCOORD1;
};

struct PSOutput
{
    float4 albedo : SV_TARGET0;
    float4 worldPos : SV_TARGET1;
    float4 worldNormal : SV_TARGET2;
};

VSOutput VS(uint vertexIndex : SV_VertexID)
{
    const Vertex vertex = g_vertices[vertexIndex];
    
    VSOutput output;
    output.pos = GetPos(vertex.position);
    output.worldPos = GetWorldPos(vertex.position);
    output.worldNormal = GetWorldNormal(vertex.normal);
    output.uv = vertex.uv;
    
    return output;
}

PSOutput PS(VSOutput input)
{
    PSOutput output;
    output.albedo = g_perMaterial.albedo;
    output.worldPos = float4(input.worldPos, 1.0f);
    output.worldNormal = float4(normalize(input.worldNormal), 0.0f);
    
    return output;
}