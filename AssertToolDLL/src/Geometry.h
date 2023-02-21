#pragma once

#include "AssertToolCommon.h"
#include "Utilities/Math.h"

namespace Xunlan::Tools
{
    using UV = Math::Vector2;
    using UVSet = std::vector<UV>;

    namespace PackedVertex
    {
        struct VertexStatic
        {
            Math::Vector3 position;
            byte reserved[3];     // with no use
            byte sign;            // 0 means -1, 2 means +1
            uint16 normal[2];
            uint16 tangent[2];
            UV uv;
        };
    }

    struct Vertex
    {
        Math::Vector3 position = {};
        Math::Vector3 normal = {};
        Math::Vector4 tangent = {};
        UV uv = {};
    };

    struct Mesh
    {
        // index means the index of the vertices to be rendered

        // Initial data
        std::vector<Math::Vector3> positions;   // positionIndex -> position (unique position in space)
        std::vector<uint32> rawIndices;       // index -> positionIndex
        std::vector<Math::Vector3> normals;     // index -> normal
        std::vector<Math::Vector4> tangents;    // index -> tangent
        std::vector<UVSet> uvSets;              // index -> uv

        // Intermediate data
        std::vector<Vertex> vertices;           // vertexIndex -> vertex (after merged)
        std::vector<uint32> indices;          // index -> vertexIndex

        // Output data
        std::string name;
        std::vector<PackedVertex::VertexStatic> packedVerticesStatic;
        float lodThreshold = -1.0f;
        uint32 lodID = UINT_MAX;
    };

    struct LodGroup
    {
        std::string name;
        std::vector<Mesh> meshes;
    };

    struct Scene
    {
        std::string name;
        std::vector<LodGroup> lodGroups;
    };

    struct GeometryImportSettings
    {
        // [0, 180], bigger means more edges will be soft
        float smoothingAngle;
        byte calculateNormals;
        byte calculateTangents;
        byte reverseHandedness;
        byte importEmbededTextures;
        byte importAnimations;
    };

    struct SceneData
    {
        byte* pBuffer;
        uint32 bufferByteSize;
        GeometryImportSettings settings;
    };

    void ProcessScene(Scene& scene, const GeometryImportSettings& settings);

    void Pack(const Scene& scene, SceneData& data);
}