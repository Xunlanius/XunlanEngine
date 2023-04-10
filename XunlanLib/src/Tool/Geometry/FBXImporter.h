#pragma once

#include "src/Function/Resource/ResourceType/MeshData.h"
#include <fbxsdk.h>
#include <vector>

namespace Xunlan
{
    class FBXImporter final
    {
    public:

        FBXImporter() = default;
        ~FBXImporter();

    public:

        bool Initialize(const char* filePath);
        bool LoadScene(MeshData& meshData);

    private:

        bool ProcessNode(fbxsdk::FbxNode* node, MeshData& meshData);
        bool ProcessMesh(fbxsdk::FbxNode* node, MeshData& meshData);
        bool ProcessSkeleton(fbxsdk::FbxNode* node, MeshData& meshData);

        void ReadNormal(fbxsdk::FbxMesh* fbxMesh, int vertexIndex, int indexCounter, Math::float3& normal);
        void ReadTangent(fbxsdk::FbxMesh* fbxMesh, int vertexIndex, int indexCounter, Math::float3& tangent);
        void ReadUV(fbxsdk::FbxMesh* fbxMesh, int vertexIndex, int uvIndex, Math::float2& uv);

    private:

        fbxsdk::FbxManager* m_fbxManager = nullptr;
        fbxsdk::FbxScene* m_fbxScene = nullptr;
    };
}