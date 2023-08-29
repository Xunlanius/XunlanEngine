#include "Common/Common.hlsli"

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 worldNormal : NORMAL;
};

struct PSOutput
{
    float4 worldPos : SV_TARGET0;
    float4 worldNormal : SV_TARGET1;
};

VSOutput VS(uint vertexIndex : SV_VertexID)
{
    const Vertex vertex = Vertices[vertexIndex];
    
    VSOutput output;
    output.pos = GetPos(vertex.position);
    output.worldPos = GetWorldPos(vertex.position);
    output.worldNormal = GetWorldNormal(vertex.normal);
    
    return output;
}

PSOutput PS(VSOutput input)
{
    PSOutput output;
    output.worldPos = float4(input.worldPos, 1.0f);
    output.worldNormal = float4(normalize(input.worldNormal), 0.0f);
    
    return output;
}