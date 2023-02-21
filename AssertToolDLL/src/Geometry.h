#pragma once

#include "AssertToolCommon.h"

namespace Xunlan::Tools
{
    using UV = Math::Vector2;
    using UVSet = std::vector<UV>;

    namespace PackedVertex
    {
        struct VertexStatic
        {
            Math::Vector3 position;
            byte_t reserved[3];     // with no use
            byte_t sign;            // 0 means -1, 2 means +1
            uint16_t normal[2];
            uint16_t tangent[2];
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
        std::vector<uint32_t> rawIndices;       // index -> positionIndex
        std::vector<Math::Vector3> normals;     // index -> normal
        std::vector<Math::Vector4> tangents;    // index -> tangent
        std::vector<UVSet> uvSets;              // index -> uv

        // Intermediate data
        std::vector<Vertex> vertices;           // vertexIndex -> vertex (after merged)
        std::vector<uint32_t> indices;          // index -> vertexIndex

        // Output data
        std::string name;
        std::vector<PackedVertex::VertexStatic> packedVerticesStatic;
        float lodThreshold = -1.0f;
        uint32_t lodID = UINT_MAX;
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
        byte_t calculateNormals;
        byte_t calculateTangents;
        byte_t reverseHandedness;
        byte_t importEmbededTextures;
        byte_t importAnimations;
    };

    struct SceneData
    {
        byte_t* pBuffer;
        uint32_t bufferByteSize;
        GeometryImportSettings settings;
    };

    void ProcessScene(Scene& scene, const GeometryImportSettings& settings);

    void Pack(const Scene& scene, SceneData& data);
}