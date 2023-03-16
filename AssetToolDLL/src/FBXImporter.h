#pragma once

#include <fbxsdk.h>
#include "AssetToolCommon.h"

namespace Xunlan::Tools
{
    struct Mesh;
    struct Scene;
    struct SceneData;
    struct GeometryImportSettings;

    class FBXImporter final
    {
    public:

        FBXImporter() = default;
        ~FBXImporter() { Release(); }

    public:

        bool Initialize(const char* filePath, Scene* scene, SceneData* sceneData);
        bool LoadScene(fbxsdk::FbxNode* root = nullptr);

        float GetSceneScale() const { return m_sceneScale; }

    private:

        bool ProcessMeshes(fbxsdk::FbxNode* node, std::vector<Mesh>& meshes, uint32 lodID, float lodThreshold);
        bool ProcessMesh(fbxsdk::FbxNode* node, std::vector<Mesh>& meshes, uint32 lodID, float lodThreshold);
        bool LoadMesh(fbxsdk::FbxMesh* fbxMesh, Mesh& mesh);
        bool ProcessLodGroup(fbxsdk::FbxNode* node);

        void Release();

    private:

        Scene* m_scene = nullptr;
        SceneData* m_sceneData = nullptr;

        fbxsdk::FbxManager* m_fbxManager = nullptr;
        fbxsdk::FbxScene* m_fbxScene = nullptr;

        float m_sceneScale = 1.0f;
    };
}