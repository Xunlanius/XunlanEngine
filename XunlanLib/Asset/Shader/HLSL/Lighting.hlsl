#include "Common/Common.hlsli"

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer g_lightingPass
{
    uint g_albedoIndex;
    uint g_posWSIndex;
    uint g_normalWSIndex;
    
    uint g_rsmIndex;
    uint g_shadowMapIndex;
};

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
    const Texture2D albedoMap = ResourceDescriptorHeap[g_albedoIndex];
    const Texture2D posWSMap = ResourceDescriptorHeap[g_posWSIndex];
    const Texture2D normalWSMap = ResourceDescriptorHeap[g_normalWSIndex];
    const Texture2D rsm = ResourceDescriptorHeap[g_rsmIndex];
    const Texture2D shadowMap = ResourceDescriptorHeap[g_shadowMapIndex];
    
    const float3 albedo = albedoMap.Sample(LinearClamp, input.uv).xyz;
    const float3 posWS = posWSMap.Sample(LinearClamp, input.uv).xyz;
    const float3 normalWS = normalize(normalWSMap.Sample(LinearClamp, input.uv).xyz);
    
    const float3 directLight = ComputeDirectionalLight(shadowMap, posWS, normalWS);
    const float3 indirectLight = rsm.Sample(LinearClamp, input.uv).xyz;
    const float3 color = albedo * directLight + indirectLight;
    
    return float4(indirectLight, 1.0f);
}