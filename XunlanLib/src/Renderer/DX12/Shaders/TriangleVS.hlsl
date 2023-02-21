struct VSOutput
{
    noperspective float4 pos : SV_POSITION;
    noperspective float2 uv : TEXCOORD0;
};

VSOutput TriangleVS(in uint vertexIndex : SV_VertexID)
{
    VSOutput output;
    
    const float2 tex = float2(uint2(vertexIndex, vertexIndex << 1) & 2);
    
    output.pos = float4(lerp(float2(-1, 1), float2(1, -1), tex), 0, 1);
    output.uv = tex;
    
    return output;
}