#include "FBXImporter.h"
#include "Geometry.h"
#include <mutex>

#if defined _DEBUG
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.0.1/lib/vs2017/x64/debug/libfbxsdk-md.lib")
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.0.1/lib/vs2017/x64/debug/libxml2-md.lib")
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.0.1/lib/vs2017/x64/debug/zlib-md.lib")
#else
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.0.1/lib/vs2017/x64/release/libfbxsdk-md.lib")
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.0.1/lib/vs2017/x64/release/libxml2-md.lib")
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.0.1/lib/vs2017/x64/release/zlib-md.lib")
#endif

namespace Xunlan::Tools
{
    namespace
    {
        std::mutex g_mutex = {};
    }

    bool FBXImporter::Initialize(const char* filePath, Scene* scene, SceneData* sceneData)
    {
        if (!filePath || !scene || !sceneData) { assert(false); return false; }

        if (m_fbxManager || m_fbxScene) Release();

        m_scene = scene;
        m_sceneData = sceneData;

        // Create a FbxManager object
        m_fbxManager = fbxsdk::FbxManager::Create();
        if (!m_fbxManager) return false;

        // Create an IOSettings object
        fbxsdk::FbxIOSettings* ios = fbxsdk::FbxIOSettings::Create(m_fbxManager, IOSROOT);
        assert(ios);
        m_fbxManager->SetIOSettings(ios);

        m_fbxScene = fbxsdk::FbxScene::Create(m_fbxManager, "Scene");
        if (!m_fbxScene) return false;

        fbxsdk::FbxImporter* importer = fbxsdk::FbxImporter::Create(m_fbxManager, "Importer");
        if (!importer) return false;
        if (!importer->Initialize(filePath, -1, m_fbxManager->GetIOSettings())) return false;
        if (!importer->Import(m_fbxScene)) return false;
        importer->Destroy();

        // Get scene scale
        m_sceneScale = (float)m_fbxScene->GetGlobalSettings().GetSystemUnit().GetConversionFactorTo(FbxSystemUnit::m);

        return true;
    }
    bool FBXImporter::LoadScene(fbxsdk::FbxNode* root)
    {
        if (!root) root = m_fbxScene->GetRootNode();

        const int numNodes = root->GetChildCount();
        for (int i = 0; i < numNodes; ++i)
        {
            fbxsdk::FbxNode* node = root->GetChild(i);
            if (!node) { assert(false); return false; }

            LODGroup& lodGroup = m_scene->lodGroups.emplace_back();
            if (!ProcessMeshes(node, lodGroup.meshes, 0, -1.0f)) { assert(false); return false; }
            if (lodGroup.meshes.size() > 0) lodGroup.name = lodGroup.meshes[0].name;
        }

        return true;
    }

    bool FBXImporter::ProcessMeshes(fbxsdk::FbxNode* node, std::vector<Mesh>& meshes, uint32 lodID, float lodThreshold)
    {
        if (!node || lodID == UINT32_MAX) { assert(false); return false; }

        bool isLODGroup = false;

        // Deal with the attribute
        {
            const fbxsdk::FbxNodeAttribute* attribute = node->GetNodeAttribute();
            if (!attribute) { assert(false); return false; }

            fbxsdk::FbxNode* child = attribute->GetNode();
            const fbxsdk::FbxNodeAttribute::EType type = attribute->GetAttributeType();

            if (type == fbxsdk::FbxNodeAttribute::eMesh)
            {
                if (!ProcessMesh(child, meshes, lodID, lodThreshold)) { assert(false); return false; }
            }
            else if (type == fbxsdk::FbxNodeAttribute::eLODGroup)
            {
                if (!ProcessLodGroup(child)) { assert(false); return false; }
                isLODGroup = true;
            }
        }

        if (isLODGroup) return true;

        // Traversal
        const int numChildren = node->GetChildCount();
        for (int i = 0; i < numChildren; ++i)
        {
            ProcessMeshes(node->GetChild(i), meshes, lodID, lodThreshold);
        }

        return true;
    }
    bool FBXImporter::ProcessMesh(fbxsdk::FbxNode* node, std::vector<Mesh>& meshes, uint32 lodID, float lodThreshold)
    {
        assert(node);

        fbxsdk::FbxMesh* fbxMesh = node->GetMesh();
        assert(fbxMesh);

        // Triangulate the fbxMesh
        {
            if (fbxMesh->RemoveBadPolygons() == -1) { assert(false); return false; }

            FbxGeometryConverter geoConverter(m_fbxManager);
            fbxMesh = (fbxsdk::FbxMesh*)geoConverter.Triangulate(fbxMesh, true);

            if (!fbxMesh) { assert(false); return false; }
            if (fbxMesh->RemoveBadPolygons() == -1) { assert(false); return false; }
        }

        // Load mesh
        Mesh& mesh = meshes.emplace_back();
        mesh.lodThreshold = lodThreshold;
        mesh.lodID = lodID;
        mesh.name = (node->GetName()[0] != '\0') ? node->GetName() : fbxMesh->GetName();

        if (!LoadMesh(fbxMesh, mesh)) { assert(false); return false; }

        return true;
    }
    bool FBXImporter::LoadMesh(fbxsdk::FbxMesh* fbxMesh, Mesh& mesh)
    {
        assert(fbxMesh);

        fbxsdk::FbxNode* const node = fbxMesh->GetNode();

        fbxsdk::FbxAMatrix transform = {};
        transform.SetT(node->GetGeometricTranslation(fbxsdk::FbxNode::eSourcePivot));
        transform.SetR(node->GetGeometricRotation(fbxsdk::FbxNode::eSourcePivot));
        transform.SetS(node->GetGeometricScaling(fbxsdk::FbxNode::eSourcePivot));
        transform = node->EvaluateGlobalTransform() * transform;
        fbxsdk::FbxAMatrix inverseTranspose = transform.Transpose().Inverse();

        const int numPolygons = fbxMesh->GetPolygonCount();
        if (numPolygons <= 0) { assert(false); return false; }

        // Get control points
        const int numControlPoints = fbxMesh->GetControlPointsCount();
        const fbxsdk::FbxVector4* controlPoints = fbxMesh->GetControlPoints();
        if (numControlPoints <= 0 || !controlPoints) { assert(false); return false; }

        // Get indices
        const int numIndices = fbxMesh->GetPolygonVertexCount();
        const int* indices = fbxMesh->GetPolygonVertices();
        if (numIndices <= 0 || !indices) { assert(false); return false; }

        mesh.indices.resize(numIndices);

        // record indices of control points that have been added
        std::vector<uint32> controlPointToVertexIndex(numControlPoints, UINT32_MAX);

        for (int i = 0; i < numIndices; ++i)
        {
            const uint32 indexOfControlPoint = (uint32)indices[i];

            if (controlPointToVertexIndex[indexOfControlPoint] == UINT32_MAX) // haven't encounter the control point before
            {
                // Record unique position of control point
                const fbxsdk::FbxVector4 pos = transform.MultT(controlPoints[indexOfControlPoint]) * m_sceneScale;
                mesh.positions.emplace_back((float)pos[0], (float)pos[1], (float)pos[2]);
                mesh.indices[i] = (uint32)mesh.positions.size() - 1;
                controlPointToVertexIndex[indexOfControlPoint] = mesh.indices[i];
            }
            else // the control point has been added
            {
                mesh.indices[i] = controlPointToVertexIndex[indexOfControlPoint];
            }
        }

        assert(mesh.indices.size() % 3 == 0);

        // Get material index per polygon
        fbxsdk::FbxLayerElementArrayTemplate<int>* materialIndices = nullptr;
        if (fbxMesh->GetMaterialIndices(&materialIndices)) // has material
        {
            for (int i = 0; i < numPolygons; ++i)
            {
                const int materialIndex = materialIndices->GetAt(i);
                mesh.materialIndices.push_back(materialIndex);

                if (std::find(mesh.materialIDs.begin(), mesh.materialIDs.end(), (uint32)materialIndex) == mesh.materialIDs.end())
                {
                    mesh.materialIDs.push_back(materialIndex);
                }
            }
        }

        const bool importNormals = !m_sceneData->settings.calculateNormals;
        const bool importTangents = !m_sceneData->settings.calculateTangents;

        if (importNormals) // Import normals if we don't calculate normals ourselves
        {
            fbxsdk::FbxArray<fbxsdk::FbxVector4> normals;

            // Calculate normals using FBX's built-in method, if normal data is not existed
            fbxMesh->GenerateNormals();
            if (fbxMesh->GetPolygonVertexNormals(normals))
            {
                const int numNormals = normals.Size();
                assert(numNormals > 0);
                mesh.normals.reserve(numNormals);

                for (int i = 0; i < numNormals; ++i)
                {
                    fbxsdk::FbxVector4 normal = inverseTranspose.MultT(normals[i]);
                    normal.Normalize();
                    mesh.normals.emplace_back((float)normal[0], (float)normal[1], (float)normal[2]);
                }
            }
            else m_sceneData->settings.calculateNormals = true;
        }

        if (importTangents) // Import tangents if we don't calculate tangents ourselves
        {
            fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxVector4>* tangents = nullptr;

            // Calculate tangents using FBX's built-in method, if tangents data is not existed
            fbxMesh->GenerateTangentsData();
            if (fbxMesh->GetTangents(&tangents))
            {
                assert(tangents && tangents->GetCount() > 0);

                const int numTangents = tangents->GetCount();
                mesh.tangents.reserve(numTangents);

                for (int i = 0; i < numTangents; ++i)
                {
                    fbxsdk::FbxVector4 tangent = tangents->GetAt(i);
                    const float handedness = (float)tangent[3];
                    tangent[3] = 0;
                    transform.MultT(tangent);
                    tangent.Normalize();
                    mesh.tangents.emplace_back((float)tangent[0], (float)tangent[1], (float)tangent[2], handedness);
                }
            }
            else m_sceneData->settings.calculateTangents = true;
        }

        // Import UVs
        fbxsdk::FbxStringList uvNames;
        fbxMesh->GetUVSetNames(uvNames);
        const int numUVSets = uvNames.GetCount();

        mesh.uvSets.resize(numUVSets);

        for (int i = 0; i < numUVSets; ++i)
        {
            fbxsdk::FbxArray<fbxsdk::FbxVector2> uvSet;
            if (fbxMesh->GetPolygonVertexUVs(uvNames.GetStringAt(i), uvSet))
            {
                const int numUVs = uvSet.Size();
                mesh.uvSets[i].reserve(numUVs);

                for (int j = 0; j < numUVs; ++j)
                {
                    const fbxsdk::FbxVector2& uv = uvSet[j];
                    mesh.uvSets[i].emplace_back((float)uv[0], (float)uv[1]);
                }
            }
        }

        return true;
    }
    bool FBXImporter::ProcessLodGroup(fbxsdk::FbxNode* node)
    {
        if (!node) { assert(false); return false; }

        fbxsdk::FbxLODGroup* fbxLODGroup = node->GetLodGroup();
        if (!fbxLODGroup) { assert(false); return false; }

        LODGroup& lodGroup = m_scene->lodGroups.emplace_back();
        lodGroup.name = node->GetName()[0] != '\0' ? node->GetName() : fbxLODGroup->GetName();

        const int numNodes = node->GetChildCount();
        if (numNodes <= 0 || fbxLODGroup->GetNumThresholds() != numNodes - 1) { assert(false); return false; }

        for (int i = 0; i < numNodes; ++i)
        {
            float lodThreshold = -1.0f;

            if (i >= 1)
            {
                fbxsdk::FbxDistance threshold = {};
                fbxLODGroup->GetThreshold(i - 1, threshold);
                lodThreshold = threshold.value() * m_sceneScale;
            }

            if (!ProcessMeshes(node->GetChild(i), lodGroup.meshes, (uint32)lodGroup.meshes.size(), lodThreshold)) { assert(false); return false; }
        }

        return true;
    }

    void FBXImporter::Release()
    {
        if (m_fbxScene)
        {
            m_fbxScene->Destroy();
            m_fbxScene = nullptr;
        }
        if (m_fbxManager)
        {
            m_fbxManager->Destroy();
            m_fbxManager = nullptr;
        }
    }

    EDITOR_INTERFACE void ImportFBX(const char* filePath, SceneData* sceneData)
    {
        if (!filePath || !sceneData) { assert(false); return; }

        Scene scene = {};

        // FBX SDK doesn't support multi-thread
        {
            std::lock_guard lock(g_mutex);

            FBXImporter fbxImporter;
            if (!fbxImporter.Initialize(filePath, &scene, sceneData)) return;
            if (!fbxImporter.LoadScene()) return;
        }

        ProcessScene(scene, sceneData->settings);
        Pack(scene, *sceneData);
    }
}