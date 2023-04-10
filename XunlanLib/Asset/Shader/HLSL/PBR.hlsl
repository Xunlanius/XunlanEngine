#include "Common.hlsli"

VertexOutput VS(uint vertexIndex : SV_VertexID)
{
    VertexOutput output;
    const Vertex vertex = Vertices[vertexIndex];
    
    output.pos = GetPos(vertex.position);
    output.worldPos = GetWorldPos(vertex.position);
    output.worldNormal = GetWorldNormal(vertex.normal);
    output.worldTangent = GetWorldTangent(vertex.tangent);
    output.uv = vertex.uv;
    
    return output;
}

float4 PS(VertexOutput input) : SV_TARGET
{
    float3 worldNormal = normalize(input.worldNormal);
    float3 worldTangent = normalize(input.worldTangent);
    
    float3 color = abs(worldNormal);
    
    return float4(color, 1.0f);
}