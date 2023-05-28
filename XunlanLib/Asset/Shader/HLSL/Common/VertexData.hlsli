#ifndef VERTEX_DATA
#define VERTEX_DATA

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 uv;
};
StructuredBuffer<Vertex> Vertices : register(t0);

#endif