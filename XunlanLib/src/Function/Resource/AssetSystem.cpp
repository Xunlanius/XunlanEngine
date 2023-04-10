#include "AssetSystem.h"

namespace Xunlan
{
    // Input:
    // struct
    // {
    //     uint32 numLODs;
    //
    //     struct MeshLOD
    //     {
    //         float lodThreshold;
    //         uint32 numSubmeshes;
    //         uint32 submeshesSize;
    //
    //         struct Submesh
    //         {
    //             uint32 numVertices;
    //             uint32 numIndices;
    //             uint32 elementType;
    //             uint32 elementSize;
    //             uint32 primitiveTopology;
    //             byte positionBuffer[numVertices * sizeof(Math::float3)];
    //             byte indexBuffer[numIndices * sizeof(uint32)];
    //             byte elementBuffer[numVertices * elementSize];
    //         } submeshes[numSubmeshes];
    //     } meshLODs[numLODs];
    // };
}