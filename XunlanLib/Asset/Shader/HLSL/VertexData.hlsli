//struct VertexInput
//{
//    float3 position : POSITION;
//    float3 normal : NORMAL;
//    float3 tangent : TANGENT;
//    float2 uv : TEXCOORD;
//};

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 uv;
};

StructuredBuffer<Vertex> Vertices : register(t0);

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 worldNormal : NORMAL;
    float3 worldTangent : TANGENT;
    float2 uv : TEXCOORD;
};