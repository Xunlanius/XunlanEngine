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