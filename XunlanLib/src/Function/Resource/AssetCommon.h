//#pragma once
//
//#include "src/Common/Common.h"
//#include "ResourceType/Data/MeshData.h"
//
//#include <vector>
//#include <memory>
//
//namespace Xunlan
//{
//    enum class AssetType : uint32
//    {
//        Unknown,
//        Mesh,
//        Texture,
//        Shader,
//        Animation,
//        Audio,
//        Skeleton,
//
//        Count,
//    };
//
//    //struct SubmeshData
//    //{
//    //    uint32 numVertices;
//    //    uint32 numIndices;
//    //    uint32 elementType;
//    //    uint32 elementSize;
//    //    uint32 primitiveTopology;
//
//    //    // numVertices * sizeof(Math::float3) +
//    //    // numIndices * sizeof(uint32) +
//    //    // numVertices * elementSize
//    //    std::vector<byte> buffer;
//    //};
//
//    //struct MeshData
//    //{
//    //    struct MeshLOD
//    //    {
//    //        float lodThreshold;
//    //        std::vector<SubmeshData> submeshes;
//    //    };
//
//    //    std::vector<MeshLOD> meshLODs;
//    //};
//
//    //REGISTER_CLASS(SubmeshData,
//    //    FIELD(numVertices),
//    //    FIELD(numIndices),
//    //    FIELD(elementType),
//    //    FIELD(elementSize),
//    //    FIELD(primitiveTopology),
//    //    FIELD(buffer))
//
//    //REGISTER_CLASS(MeshData::MeshLOD,
//    //    FIELD(lodThreshold),
//    //    FIELD(submeshes))
//
//    //REGISTER_CLASS(MeshData,
//    //    FIELD(meshLODs))
//
//    enum class PrimitiveTopology : uint32
//    {
//        PointList = 1,
//        LineList,
//        LineStrip,
//        TriangleList,
//        TriangleStrip,
//
//        Count,
//    };
//
//    struct Mesh final
//    {
//        /*struct LODLevel final
//        {
//            float lodThreshold;
//            uint16 offset;
//            uint16 count;
//        };
//
//        std::vector<LODLevel> lods;
//        uint32 GetNumLODs() const { return (uint32)lods.size(); }*/
//
//        std::vector<LongID> submeshIDs;
//        uint32 GetNumSubmeshes() const { return (uint32)submeshIDs.size(); }
//    };
//
//    enum class ShaderType : uint32
//    {
//        Vertex,
//        Hull,
//        Domain,
//        Geometry,
//        Pixel,
//        Compute,
//        Amplification,
//        Mesh,
//
//        Count,
//    };
//
//    struct ShaderFlags
//    {
//        enum Value : uint32
//        {
//            Vertex = 1,
//            Hull = 2,
//            Domain = 4,
//            Geometry = 8,
//            Pixel = 16,
//            Compute = 32,
//            Amplification = 64,
//            Mesh = 128,
//        };
//    };
//
//    struct ShaderFileInfo final
//    {
//        const char* fileName = nullptr;
//        const char* functionName = nullptr;
//        ShaderType type = (ShaderType)-1;
//    };
//
//    struct CompiledShader final
//    {
//        std::vector<byte> byteCode;
//    };
//
//    REGISTER_CLASS(CompiledShader,
//        FIELD(byteCode))
//
//    enum class MaterialType
//    {
//        Opaque,
//
//        Count,
//    };
//
//    struct MaterialInitDesc final
//    {
//        MaterialType type;
//        LongID shaderIDs[(uint32)ShaderType::Count] = { INVALID_LONG_ID, INVALID_LONG_ID, INVALID_LONG_ID, INVALID_LONG_ID, INVALID_LONG_ID, INVALID_LONG_ID, INVALID_LONG_ID, INVALID_LONG_ID };
//        std::vector<LongID> textureIDs;
//    };
//}