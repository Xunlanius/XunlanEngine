#include "Common.hlsli"

VertexOutput VS(uint vertexIndex : SV_VertexID)
{
    VertexOutput output;
    
    const Vertex vertex = Vertices[vertexIndex];
    output.pos = float4(vertex.position, 1.0f);
    output.uv = vertex.uv;
    
    return output;
}

float4 PS(VertexOutput input) : SV_TARGET
{
    Texture2D baseColor = ResourceDescriptorHeap[BaseColorIndex];
    float4 color = baseColor.Sample(LinearClamp, input.uv);
    return float4(color.xyz, 1.0f);
}