#include "Common/Common.hlsli"

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

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
    Texture2D baseColor = ResourceDescriptorHeap[g_meshTextures.albedoIndex];
    float4 color = baseColor.Sample(LinearClamp, input.uv);
    return float4(color.xyz, 1.0f);
}