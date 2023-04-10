#pragma once

#include "src/Common/Common.h"
#include "src/Utility/Reflection/Reflection.h"
#include "src/Utility/MathTypes.h"
#include <vector>

namespace Xunlan
{
    struct VertexData final
    {
        Math::float3 position;
        Math::float3 normal;
        Math::float3 tangent;
        Math::float2 uv;
    };

    REGISTER_CLASS(VertexData,
        FIELD(position),
        FIELD(normal),
        FIELD(tangent),
        FIELD(uv))

    struct SubmeshData final
    {
        std::vector<VertexData> vertexBuffer;
        std::vector<uint32> indexBuffer;
    };

    REGISTER_CLASS(SubmeshData,
        FIELD(vertexBuffer),
        FIELD(indexBuffer))

    struct MeshData final
    {
        std::vector<SubmeshData> submeshes;
    };

    REGISTER_CLASS(MeshData,
        FIELD(submeshes))
}