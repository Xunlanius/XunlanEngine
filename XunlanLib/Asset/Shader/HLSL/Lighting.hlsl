#include "Common/Common.hlsli"

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOutput VS(uint vertexIndex : SV_VertexID)
{
    const Vertex vertex = Vertices[vertexIndex];
    
    VSOutput output;
    output.pos = float4(vertex.position, 1.0f);
    output.uv = vertex.uv;
    
    return output;
}

float4 PS(VSOutput input) : SV_TARGET
{
    const Texture2D albedoMap = ResourceDescriptorHeap[g_GBuffer.albedoIndex];
    const Texture2D positionMap = ResourceDescriptorHeap[g_GBuffer.posWSIndex];
    const Texture2D normalMap = ResourceDescriptorHeap[g_GBuffer.normalWSIndex];
    
    const float3 albedo = albedoMap.Sample(LinearClamp, input.uv).xyz;
    const float3 worldPos = positionMap.Sample(LinearClamp, input.uv).xyz;
    const float3 worldNormal = normalize(normalMap.Sample(LinearClamp, input.uv).xyz);
    
    const float3 color = albedo * ComputeDirectionalLight(worldPos, worldNormal);
    
    return float4(color.xyz, 1.0f);
}