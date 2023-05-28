#include "Common/Common.hlsli"

struct VertexOutput
{
    float4 pos : SV_POSITION;
};

VertexOutput VS(uint vertexIndex : SV_VertexID)
{
    VertexOutput output;
    const Vertex vertex = Vertices[vertexIndex];
    
    const float3 worldPos = GetWorldPos(vertex.position);
    output.pos = mul(g_perFrame.directionLights[0].viewProj, float4(worldPos, 1.0f));
    
    return output;
}