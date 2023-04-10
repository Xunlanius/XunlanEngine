struct PostProcessConstants
{
    uint GPassMainBufferIndex;
};

ConstantBuffer<PostProcessConstants> ShaderParams : register(b1);

float4 PostProcessPS(in noperspective float4 pos : SV_Position, in noperspective float2 uv : TEXCOORD0) : SV_TARGET0
{
    Texture2D GPassMain = ResourceDescriptorHeap[ShaderParams.GPassMainBufferIndex];
    float4 color = float4(GPassMain[pos.xy].xyz, 1.0f);
    
    return color;
}