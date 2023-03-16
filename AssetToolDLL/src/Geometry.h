#pragma once

#include "AssetToolCommon.h"
#include "Utility/Math.h"

namespace Xunlan::Tools
{
    using UV = Math::Vector2;
    using UVSet = std::vector<UV>;

    namespace Element
    {
        struct ElementType
        {
            enum Type : uint32
            {
                POSITION_ONLY = 0,
                NORMAL = 1,
                NORMAL_TEXTURE = 3,
                COLOR = 4,
                SKELETAL = 8,

                SKELETAL_NORMAL = SKELETAL | NORMAL,
                SKELETAL_COLOR = SKELETAL | COLOR,
                SKELETAL_NORMAL_TEXTURE = SKELETAL | NORMAL_TEXTURE,
                SKELETAL_NORMAL_COLOR = SKELETAL | NORMAL | COLOR,
                SKELETAL_NORMAL_TEXTURE_COLOR = SKELETAL | NORMAL_TEXTURE | COLOR,
            };
        };

        struct Normal
        {
            byte color[3];
            byte sign;
            uint16 normal[2];
        };

        struct NormalTexture
        {
            byte color[3];
            byte sign;
            uint16 normal[2];
            uint16 tangent[2];
            Math::Vector2 uv;
        };

        struct Color
        {
            byte color[3];
            byte pad;
        };

        struct Skeletal
        {
            byte jointWeights[3];
            byte pad;
            uint16 jointIndices[4];
        };

        struct SkeletalNormal
        {
            byte jointWeights[3];
            byte sign;
            uint16 jointIndices[4];
            uint16 normal[2];
        };

        struct SkeletalColor
        {
            byte jointWeights[3];
            byte pad;
            uint16 jointIndices[4];
            byte color[3];
            byte pad2;
        };

        struct SkeletalNormalTexture
        {
            byte jointWeights[3];
            byte sign;
            uint16 jointIndices[4];
            uint16 normal[2];
            uint16 tangent[2];
            Math::Vector2 uv;
        };

        struct SkeletalNormalColor
        {
            byte jointWeights[3];
            byte sign;
            uint16 jointIndices[4];
            uint16 normal[2];
            byte color[3];
            byte pad;
        };

        struct SkeletalNormalTextureColor
        {
            byte jointWeights[3];
            byte sign;
            uint16 jointIndices[4];
            uint16 normal[2];
            uint16 tangent[2];
            Math::Vector2 uv;
            byte color[3];
            byte pad;
        };
    }

    struct Vertex
    {
        Math::Vector3 position = {};
        Math::Vector3 normal = {};
        Math::Vector4 tangent = {};
        UV uv = {};
        Math::Vector4 jointWeights = {};
        Math::U32Vector4 jointIndices = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
        byte red = {};
        byte green = {};
        byte blue = {};
        byte pad = {};
    };

    struct Mesh
    {
        std::vector<Math::Vector3> positions;   // vertexIndex -> position (unique position in space)
        std::vector<Math::Vector3> normals;     // index -> normal
        std::vector<Math::Vector4> tangents;    // index -> tangent
        std::vector<UVSet> uvSets;              // UVSet: index -> uv
        std::vector<Math::Vector3> colors;      // index -> color

        std::vector<uint32> materialIndices;    // polygonIndex -> materialIndex
        std::vector<uint32> materialIDs;        // materialIndex used in this mesh

        std::vector<uint32> indices;            // index -> vertexIndex
        std::vector<Vertex> vertices;           // vertexIndex -> vertexData

        std::string name;
        Element::ElementType::Type elementsType = {};
        std::vector<byte> positionBuffer;
        std::vector<byte> elementBuffer;

        float lodThreshold = -1.0f;
        uint32 lodID = UINT_MAX;
    };

    struct LODGroup
    {
        std::string name;
        std::vector<Mesh> meshes;
    };

    struct Scene
    {
        std::string name;
        std::vector<LODGroup> lodGroups;
    };

    struct GeometryImportSettings
    {
        // [0, 180], bigger means more edges will be soft
        float smoothingAngle;
        bool calculateNormals = false;
        bool calculateTangents = true;
        bool reverseHandedness = false;
        bool importEmbededTextures = true;
        bool importAnimations = true;
    };

    struct SceneData
    {
        byte* pBuffer;
        uint32 bufferByteSize;
        GeometryImportSettings settings;
    };

    void ProcessScene(Scene& scene, const GeometryImportSettings& settings);

    // Pack the Scene into SceneData
    void Pack(const Scene& scene, SceneData& data);
}