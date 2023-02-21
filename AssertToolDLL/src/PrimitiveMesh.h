#pragma once

#include "AssertToolCommon.h"
#include "Geometry.h"

namespace Xunlan::Tools
{
    enum class PrimitiveMeshType
    {
        Plane,
        Cube,
        UVSphere,
        IcoSphere,
        Cylinder,
        Capsule,

        Count
    };

    struct PrimitiveMeshInitInfo
    {
        PrimitiveMeshType meshType;
        uint32 segments[3] = { 1, 1, 1 };
        Math::Vector3 size = { 1, 1, 1 };
        uint32 lod = 0;
    };
}