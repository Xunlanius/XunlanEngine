#include "Common/Common.hlsli"

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 worldNormal : NORMAL;
    float3 worldTangent : TANGENT;
    float2 uv : TEXCOORD;
};

VertexOutput TestShaderVS(in uint VertexIndex : SV_VertexID)
{
    VertexOutput output;
    
    Vertex vertex = Vertices[VertexIndex];
    
    output.pos = GetPos(vertex.position);
    output.worldPos = GetWorldPos(vertex.position);
    output.worldNormal = GetWorldNormal(vertex.normal);
    output.worldTangent = GetWorldTangent(vertex.tangent);
    output.uv = vertex.uv;
    
    return output;
}

float4 TestShaderPS(VertexOutput input) : SV_TARGET
{
    float3 worldNormal = normalize(input.worldNormal);
    float3 worldTangent = normalize(input.worldTangent);
    
    float3 color = ComputeDirectionLight(input.worldPos, worldNormal);
    
    return float4(color, 1.0f);
}