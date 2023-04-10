#include "FBXImporter.h"
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

namespace Xunlan
{
    FBXImporter::~FBXImporter()
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

    bool FBXImporter::Initialize(const char* filePath)
    {
        if (!filePath) { assert(false); return false; }

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

        //fbxsdk::FbxAxisSystem::DirectX.ConvertScene(m_fbxScene);

        return true;
    }
    bool FBXImporter::LoadScene(MeshData& meshData)
    {
        fbxsdk::FbxNode* root = m_fbxScene->GetRootNode();
        const int numNodes = root->GetChildCount();

        for (int i = 0; i < numNodes; ++i)
        {
            if (!ProcessNode(root->GetChild(i), meshData)) return false;
        }

        return true;
    }

    bool FBXImporter::ProcessNode(fbxsdk::FbxNode* node, MeshData& meshData)
    {
        assert(node);
        const fbxsdk::FbxNodeAttribute* attribute = node->GetNodeAttribute();

        if (attribute)
        {
            switch (attribute->GetAttributeType())
            {
                case fbxsdk::FbxNodeAttribute::eMesh:
                {
                    if (!ProcessMesh(node, meshData)) return false;
                    break;
                }
                case fbxsdk::FbxNodeAttribute::eSkeleton:
                {
                    if (!ProcessSkeleton(node, meshData)) return false;
                    break;
                }
            }
        }

        const int numNodes = node->GetChildCount();

        for (int i = 0; i < numNodes; ++i)
        {
            if (!ProcessNode(node->GetChild(i), meshData)) return false;
        }

        return true;
    }
    bool FBXImporter::ProcessMesh(fbxsdk::FbxNode* node, MeshData& meshData)
    {
        using namespace DirectX;

        fbxsdk::FbxMesh* fbxMesh = node->GetMesh();
        if (!fbxMesh) return false;

        if (!fbxMesh->IsTriangleMesh()) // Triangulate the fbxMesh
        {
            if (fbxMesh->RemoveBadPolygons() == -1) { assert(false); return false; }

            FbxGeometryConverter geoConverter(m_fbxManager);
            fbxMesh = (fbxsdk::FbxMesh*)geoConverter.Triangulate(fbxMesh, true);

            if (!fbxMesh) { assert(false); return false; }
            if (fbxMesh->RemoveBadPolygons() == -1) { assert(false); return false; }
        }

        const int numVertices = fbxMesh->GetControlPointsCount();
        const int numPolygons = fbxMesh->GetPolygonCount();
        const int numIndices = fbxMesh->GetPolygonVertexCount();
        assert(numPolygons * 3 == numIndices);

        std::vector<Math::float3> positionBuffer(numVertices);
        std::vector<uint32> indexBuffer(numIndices);
        std::vector<Math::float3> normals(numIndices);
        std::vector<Math::float3> tangents(numIndices);
        std::vector<Math::float2> uvs(numIndices);

        const fbxsdk::FbxVector4* ctrlPoints = fbxMesh->GetControlPoints();
        for (int i = 0; i < numVertices; ++i)
        {
            const fbxsdk::FbxVector4& ctrlPoint = ctrlPoints[i];
            positionBuffer[i].x = (float)ctrlPoint[0];
            positionBuffer[i].y = (float)ctrlPoint[1];
            positionBuffer[i].z = (float)ctrlPoint[2];
        }

        int indexCounter = 0;

        for (int i = 0; i < numPolygons; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                const int vertexIndex = fbxMesh->GetPolygonVertex(i, j);
                indexBuffer[indexCounter] = vertexIndex;

                ReadNormal(fbxMesh, vertexIndex, indexCounter, normals[indexCounter]);
                ReadTangent(fbxMesh, vertexIndex, indexCounter, tangents[indexCounter]);
                ReadUV(fbxMesh, vertexIndex, fbxMesh->GetTextureUVIndex(i, j), uvs[indexCounter]);

                ++indexCounter;
            }
        }

        SubmeshData& submesh = meshData.submeshes.emplace_back();
        submesh.vertexBuffer.reserve(numIndices);
        submesh.indexBuffer.resize(numIndices, UINT32_MAX);

        std::vector<std::vector<uint32>> vertexToIndices(numVertices);
        for (int i = 0; i < numIndices; ++i)
        {
            const int vertexIndex = indexBuffer[i];
            vertexToIndices[vertexIndex].push_back(i);
        }

        for (int i = 0; i < numVertices; ++i)
        {
            const std::vector<uint32>& indices = vertexToIndices[i];
            const size_t count = indices.size();

            for (uint32 j = 0; j < count; ++j)
            {
                const uint32 index = indices[j];
                if (submesh.indexBuffer[index] != UINT32_MAX) continue;

                VertexData& vertex = submesh.vertexBuffer.emplace_back();
                submesh.indexBuffer[index] = (uint32)submesh.vertexBuffer.size() - 1;

                vertex.position = positionBuffer[i];

                XMVECTOR normal1 = XMLoadFloat3(&normals[index]);
                XMVECTOR tangent1 = XMLoadFloat3(&tangents[index]);
                XMVECTOR uv1 = XMLoadFloat2(&uvs[index]);

                for (uint32 k = j + 1; k < count; ++k)
                {
                    const uint32 index2 = indices[k];
                    if (submesh.indexBuffer[index2] != UINT32_MAX) continue;

                    XMVECTOR normal2 = XMLoadFloat3(&normals[index2]);
                    XMVECTOR tangent2 = XMLoadFloat3(&tangents[index2]);
                    XMVECTOR uv2 = XMLoadFloat2(&uvs[index2]);

                    XMVECTOR epsilon3 = XMVectorSet(Math::EPSILON, Math::EPSILON, Math::EPSILON, 0.0f);
                    XMVECTOR epsilon2 = XMVectorSet(Math::EPSILON, Math::EPSILON, 0.0f, 0.0f);

                    bool result1 = XMVector3NearEqual(normal1, normal2, epsilon3);
                    bool result2 = XMVector3NearEqual(tangent1, tangent2, epsilon3);
                    bool result3 = XMVector2NearEqual(uv1, uv2, epsilon2);

                    if (result1 && result2 && result3)
                    {
                        normal1 = XMVector3Normalize(normal1 + normal2);
                        tangent1 = XMVector3Normalize(tangent1 + tangent2);
                        submesh.indexBuffer[index2] = submesh.indexBuffer[index];
                    }
                }

                XMStoreFloat3(&vertex.normal, normal1);
                XMStoreFloat3(&vertex.tangent, tangent1);
                XMStoreFloat2(&vertex.uv, uv1);
            }
        }

        return true;
    }
    bool FBXImporter::ProcessSkeleton(fbxsdk::FbxNode* node, MeshData& meshData) { return false; }

    void FBXImporter::ReadNormal(fbxsdk::FbxMesh* fbxMesh, int vertexIndex, int indexCounter, Math::float3& normal)
    {
        if (fbxMesh->GetElementNormalCount() <= 0) return;

        const fbxsdk::FbxGeometryElementNormal* normals = fbxMesh->GetElementNormal();

        int index = -1;
        switch (normals->GetMappingMode())
        {
            case FbxGeometryElement::eByControlPoint: index = vertexIndex; break;
            case FbxGeometryElement::eByPolygonVertex: index = indexCounter; break;
            default: assert(false);
        }

        fbxsdk::FbxVector4 fbxNormal = {};

        switch (normals->GetReferenceMode())
        {
            case FbxGeometryElement::eDirect:
            {
                fbxNormal = normals->GetDirectArray().GetAt(index);
                break;
            }

            case FbxGeometryElement::eIndexToDirect:
            {
                const int id = normals->GetIndexArray().GetAt(index);
                fbxNormal = normals->GetDirectArray().GetAt(id);
                break;
            }

            default: assert(false);
        }

        fbxNormal.Normalize();
        normal = { (float)fbxNormal[0], (float)fbxNormal[1], (float)fbxNormal[2] };
    }
    void FBXImporter::ReadTangent(fbxsdk::FbxMesh* fbxMesh, int vertexIndex, int indexCounter, Math::float3& tangent)
    {
        if (fbxMesh->GetElementTangentCount() <= 0) return;

        const fbxsdk::FbxGeometryElementTangent* tangents = fbxMesh->GetElementTangent();

        int index = -1;
        switch (tangents->GetMappingMode())
        {
            case FbxGeometryElement::eByControlPoint: index = vertexIndex; break;
            case FbxGeometryElement::eByPolygonVertex: index = indexCounter; break;
            default: assert(false);
        }

        fbxsdk::FbxVector4 fbxTangent = {};

        switch (tangents->GetReferenceMode())
        {
            case FbxGeometryElement::eDirect:
            {
                fbxTangent = tangents->GetDirectArray().GetAt(index);
                break;
            }

            case FbxGeometryElement::eIndexToDirect:
            {
                const int id = tangents->GetIndexArray().GetAt(index);
                fbxTangent = tangents->GetDirectArray().GetAt(id);
                break;
            }

            default: assert(false);
        }

        fbxTangent.Normalize();
        tangent = { (float)fbxTangent[0], (float)fbxTangent[1], (float)fbxTangent[2] };
    }
    void FBXImporter::ReadUV(fbxsdk::FbxMesh* fbxMesh, int vertexIndex, int uvIndex, Math::float2& uv)
    {
        if (fbxMesh->GetElementUVCount() <= 0) return;

        fbxsdk::FbxGeometryElementUV* uvs = fbxMesh->GetElementUV(0);
        fbxsdk::FbxVector2 fbxUV = {};

        switch (uvs->GetMappingMode())
        {
            case FbxGeometryElement::eByControlPoint:
            {
                switch (uvs->GetReferenceMode())
                {
                    case FbxGeometryElement::eDirect:
                    {
                        fbxUV = uvs->GetDirectArray().GetAt(vertexIndex);
                        break;
                    }

                    case FbxGeometryElement::eIndexToDirect:
                    {
                        const int id = uvs->GetIndexArray().GetAt(vertexIndex);
                        fbxUV = uvs->GetDirectArray().GetAt(id);
                        break;
                    }
                }
                break;
            }

            case FbxGeometryElement::eByPolygonVertex:
            {
                switch (uvs->GetReferenceMode())
                {
                    case FbxGeometryElement::eDirect:
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        fbxUV = uvs->GetDirectArray().GetAt(uvIndex);
                        break;
                    }
                }
            }
        }

        uv = { (float)fbxUV[0], (float)fbxUV[1] };
    }
}