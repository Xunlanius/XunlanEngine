#include "Common/Fractal.hlsli"

struct PostProcessConstants
{
    float width;
    float height;
    uint frame;
};

ConstantBuffer<PostProcessConstants> ShaderParams : register(b1);

float4 TrianglePS(in noperspective float4 pos : SV_Position, in noperspective float2 uv : TEXCOORD0) : SV_TARGET0
{
    const float2 invDim = float2(1.0f / ShaderParams.width, 1.0f / ShaderParams.height);
    const float2 normalizedUV = pos.xy * invDim;
    
    //float3 color = DrawMandelBrot(normalizedUV);
    float3 color = DrawJuliaSet(normalizedUV, ShaderParams.frame);
    
    return float4(float3(color.z, color.x, 1.0f) * color.x, 1.0f);
}