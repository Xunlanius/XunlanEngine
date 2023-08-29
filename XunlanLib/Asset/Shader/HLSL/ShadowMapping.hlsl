#include "Common/Common.hlsli"

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 posWS : TEXCOORD0;
    float3 normalWS : NORMAL;
    float2 uv : TEXCOORD1;
};

struct PSOutput
{
    float4 flux : SV_TARGET0;
    float4 posWS : SV_TARGET1;
    float4 normalWS : SV_TARGET2;
};

VSOutput VS(uint vertexIndex : SV_VertexID)
{
    const Vertex vertex = Vertices[vertexIndex];
    
    const float3 worldPos = GetWorldPos(vertex.position);
    
    VSOutput output;
    output.pos = mul(g_perFrame.directionalLight.viewProj, float4(worldPos, 1.0f));
    output.posWS = worldPos;
    output.normalWS = GetWorldNormal(vertex.normal);
    output.uv = vertex.uv;
    
    return output;
}

PSOutput PS(VSOutput input)
{
    PSOutput output;
    output.flux = float4(g_perMaterial.albedo.xyz * g_perFrame.directionalLight.color, 1.0f);
    output.posWS = float4(input.posWS, 1.0f);
    output.normalWS = float4(normalize(input.normalWS), 0.0f);
    
    return output;
}