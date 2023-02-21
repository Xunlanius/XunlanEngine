#include "Geometry.h"

namespace Xunlan::Tools
{
    namespace
    {
        using namespace Math;

        void RecalculateNormals(Mesh& mesh)
        {
            using namespace DirectX;

            const uint32 numIndices = (uint32)mesh.rawIndices.size();
            assert(numIndices > 0);
            assert(numIndices % 3 == 0);

            mesh.normals.resize(numIndices);

            for (uint32 i = 0; i < numIndices;)
            {
                const uint32 index0 = mesh.rawIndices[i];
                const uint32 index1 = mesh.rawIndices[i + 1];
                const uint32 index2 = mesh.rawIndices[i + 2];

                XMVECTOR vertex0 = XMLoadFloat3(&mesh.positions[index0]);
                XMVECTOR vertex1 = XMLoadFloat3(&mesh.positions[index1]);
                XMVECTOR vertex2 = XMLoadFloat3(&mesh.positions[index2]);

                XMVECTOR normal = XMVector3Normalize(XMVector3Cross(vertex1 - vertex0, vertex2 - vertex0));

                XMStoreFloat3(&mesh.normals[i], normal);
                mesh.normals[i + 1] = mesh.normals[i];
                mesh.normals[i + 2] = mesh.normals[i];

                i += 3;
            }
        }

        // Construct the vertices (merged by normal)
        // smoothingAngle: angle between faces
        void ProcessNormals(Mesh& mesh, float smoothingAngle)
        {
            using namespace DirectX;

            assert(smoothingAngle >= 0 && smoothingAngle <= 180.0f);

            // n is the angle between normals
            // If cos(n) >= cosAlpha, the edge is soft
            // else cos(n) < cosAlpha, the edge is hard
            const float cosAlpha = XMScalarCos(XM_PI * (1 - smoothingAngle / 180.0f));
            const bool absolutelySoft = XMScalarNearEqual(smoothingAngle, 0.0f, Epsilon);
            const bool absolutelyHard = XMScalarNearEqual(smoothingAngle, 180.0f, Epsilon);

            const uint32 numPos = (uint32)mesh.positions.size();
            const uint32 numIndices = (uint32)mesh.rawIndices.size();
            assert(numPos > 0);
            assert(numIndices > 0);
            assert(numIndices % 3 == 0);

            std::vector<std::vector<uint32>> idxRef(numPos);
            for (uint32 i = 0; i < numIndices; ++i)
            {
                idxRef[mesh.rawIndices[i]].push_back(i);
            }

            mesh.indices.resize(numIndices, UINT_MAX);

            for (const auto& refs : idxRef)
            {
                uint32 numRefs = (uint32)refs.size();
                for (uint32 j = 0; j < numRefs; ++j)
                {
                    if (mesh.indices[refs[j]] != UINT_MAX) continue;

                    mesh.indices[refs[j]] = (uint32)mesh.vertices.size();
                    Vertex& vertex = mesh.vertices.emplace_back();

                    // Position
                    vertex.position = mesh.positions[mesh.rawIndices[refs[j]]];

                    // Normal
                    if (absolutelyHard)
                        vertex.normal = mesh.normals[refs[j]];
                    else
                    {
                        // If the edge is hard, merge the vertex
                        XMVECTOR n1 = XMLoadFloat3(&mesh.normals[refs[j]]);

                        for (uint32 k = j + 1; k < numRefs; ++k)
                        {
                            if (mesh.indices[refs[k]] != UINT_MAX) continue;

                            XMVECTOR n2 = XMLoadFloat3(&mesh.normals[refs[k]]);

                            // cosTheta of the angle between normals
                            // cos(angle) = dot(n1, n2) / (||n1|| * ||n2||)
                            float cosTheta = 0;
                            if (!absolutelySoft)
                                XMStoreFloat(&cosTheta, XMVector3Dot(n1, n2));

                            if (absolutelySoft || cosTheta >= cosAlpha)
                            {
                                n1 = XMVector3Normalize(n1 + n2);
                                mesh.indices[refs[k]] = mesh.indices[refs[j]];
                            }
                        }
                        XMStoreFloat3(&vertex.normal, n1);
                    }
                }
            }
        }
        // Construct the vertices (merged by uv)
        void ProcessUVSets(Mesh& mesh)
        {
            using namespace DirectX;

            std::vector<Vertex> oldVertices;
            oldVertices.swap(mesh.vertices);
            std::vector<uint32> oldIndices(mesh.indices.size(), UINT_MAX);
            oldIndices.swap(mesh.indices);

            const uint32 numVerices = (uint32)oldVertices.size();
            const uint32 numIndices = (uint32)oldIndices.size();
            assert(numVerices > 0);
            assert(numIndices > 0);
            assert(numIndices % 3 == 0);

            std::vector<std::vector<uint32>> idxRef(numVerices);

            for (uint32 i = 0; i < numIndices; ++i)
            {
                idxRef[oldIndices[i]].push_back(i);
            }

            for (const auto& refs : idxRef)
            {
                uint32 numRefs = (uint32)refs.size();
                for (uint32 j = 0; j < numRefs; ++j)
                {
                    if (mesh.indices[refs[j]] != UINT_MAX) continue;

                    mesh.indices[refs[j]] = (uint32)mesh.vertices.size();
                    Vertex& vertex = oldVertices[oldIndices[refs[j]]];
                    vertex.uv = mesh.uvSets[0][refs[j]];
                    mesh.vertices.push_back(vertex);

                    for (uint32 k = j + 1; k < numRefs; ++k)
                    {
                        if (mesh.indices[refs[k]] != UINT_MAX) continue;

                        const UV& uv = mesh.uvSets[0][refs[k]];
                        if (XMScalarNearEqual(vertex.uv.x, uv.x, Epsilon) && XMScalarNearEqual(vertex.uv.y, uv.y, Epsilon))
                        {
                            mesh.indices[refs[k]] = mesh.indices[refs[j]];
                        }
                    }
                }
            }
        }

        void PackVerticesStatic(Mesh& mesh)
        {
            const uint32 numVertices = (uint32)mesh.vertices.size();
            assert(numVertices > 0);

            mesh.packedVerticesStatic.reserve(numVertices);

            for (const Vertex& vertex : mesh.vertices)
            {
                const byte sign = (byte)(vertex.normal.z > 0.0f) << 1;
                const uint16 normalX = (uint16)PackFloat<16>(vertex.normal.x, -1.0f, 1.0f);
                const uint16 normalY = (uint16)PackFloat<16>(vertex.normal.y, -1.0f, 1.0f);

                // TODO: Pack tangents

                auto& packedVertex = mesh.packedVerticesStatic.emplace_back();
                packedVertex.position = vertex.position;
                packedVertex.sign = sign;
                packedVertex.normal[0] = normalX;
                packedVertex.normal[1] = normalY;
                // TODO: tangents
                packedVertex.uv = vertex.uv;
            }
        }

        void ProcessVertices(Mesh& mesh, const GeometryImportSettings& settings)
        {
            assert(mesh.rawIndices.size() > 0);
            assert(mesh.rawIndices.size() % 3 == 0);

            if (settings.calculateNormals || mesh.normals.empty())
                RecalculateNormals(mesh);

            ProcessNormals(mesh, settings.smoothingAngle);

            if (!mesh.uvSets.empty())
                ProcessUVSets(mesh);

            PackVerticesStatic(mesh);
        }

        size_t GetMeshByteSize(const Mesh& mesh)
        {
            const size_t nameBufferByteSize = mesh.name.size();
            const size_t vertexBufferByteSize = sizeof(PackedVertex::VertexStatic) * mesh.packedVerticesStatic.size();
            const size_t indexBufferByteSize = sizeof(uint32) * mesh.indices.size();

            const size_t byteSize = {
                sizeof(uint32) +      // name length
                nameBufferByteSize +    // name string
                sizeof(uint32) +      // LOD ID
                sizeof(uint32) +      // vertex byte size
                sizeof(uint32) +      // number of vertices
                sizeof(uint32) +      // index byte size
                sizeof(uint32) +      // number of indices
                sizeof(float) +         // LOD threshold
                vertexBufferByteSize +  // vertex buffer byte size
                indexBufferByteSize     // index buffer byte size
            };

            return byteSize;
        }
        size_t GetSceneByteSize(const Scene& scene)
        {
            constexpr size_t numByteSize = sizeof(uint32);
            size_t byteSize = numByteSize + scene.name.size() + numByteSize;

            for (const auto& lod : scene.lodGroups)
            {
                byteSize += numByteSize + lod.name.size() + numByteSize;

                for (const auto& mesh : lod.meshes)
                {
                    byteSize += GetMeshByteSize(mesh);
                }
            }

            return byteSize;
        }

        void PackMeshData(const Mesh& mesh, byte*& pDst)
        {
            uint32 src = 0;

            const size_t vertexBufferByteSize = sizeof(PackedVertex::VertexStatic) * mesh.packedVerticesStatic.size();
            const size_t indexBufferByteSize = sizeof(uint32) * mesh.indices.size();

            // mesh name
            src = (uint32)mesh.name.size();
            memcpy(pDst, &src, sizeof(uint32));   pDst += sizeof(uint32);
            memcpy(pDst, mesh.name.data(), src);    pDst += src;

            // LOD ID
            memcpy(pDst, &mesh.lodID, sizeof(uint32));   pDst += sizeof(uint32);

            // vertex byte size
            src = sizeof(PackedVertex::VertexStatic);
            memcpy(pDst, &src, sizeof(uint32));   pDst += sizeof(uint32);
            // number of vertices
            src = (uint32)mesh.packedVerticesStatic.size();
            memcpy(pDst, &src, sizeof(uint32));   pDst += sizeof(uint32);

            // index byte size
            src = sizeof(uint32);
            memcpy(pDst, &src, sizeof(uint32));   pDst += sizeof(uint32);
            // number of indices
            src = (uint32)mesh.indices.size();
            memcpy(pDst, &src, sizeof(uint32));   pDst += sizeof(uint32);

            // LOD threshold
            memcpy(pDst, &mesh.lodThreshold, sizeof(float));   pDst += sizeof(float);

            // vertex buffer byte size
            memcpy(pDst, mesh.packedVerticesStatic.data(), vertexBufferByteSize);   pDst += vertexBufferByteSize;
            // index buffer byte size
            memcpy(pDst, mesh.indices.data(), indexBufferByteSize);                 pDst += indexBufferByteSize;
        }
    }

    void ProcessScene(Scene& scene, const GeometryImportSettings& settings)
    {
        for (auto& lodGroup : scene.lodGroups)
        {
            for (auto& mesh : lodGroup.meshes)
            {
                ProcessVertices(mesh, settings);
            }
        }
    }

    // Pack the Scene into SceneData
    void Pack(const Scene& scene, SceneData& data)
    {
        const size_t sceneByteSize = GetSceneByteSize(scene);
        data.bufferByteSize = (uint32)sceneByteSize;
        data.pBuffer = (byte*)CoTaskMemAlloc(sceneByteSize);
        assert(data.pBuffer);

        byte* pDst = data.pBuffer;
        uint32 src = 0;

        // scene name
        src = (uint32)scene.name.size();
        memcpy(pDst, &src, sizeof(uint32));   pDst += sizeof(uint32);
        memcpy(pDst, scene.name.data(), src);   pDst += src;

        // number of LODs
        src = (uint32)scene.lodGroups.size();
        memcpy(pDst, &src, sizeof(uint32));   pDst += sizeof(uint32);

        for (const LodGroup& lod : scene.lodGroups)
        {
            // LOD name
            src = (uint32)lod.name.size();
            memcpy(pDst, &src, sizeof(uint32));   pDst += sizeof(uint32);
            memcpy(pDst, lod.name.data(), src);     pDst += src;

            // number of Meshes
            src = (uint32)lod.meshes.size();
            memcpy(pDst, &src, sizeof(uint32));   pDst += sizeof(uint32);

            for (const Mesh& mesh : lod.meshes)
            {
                PackMeshData(mesh, pDst);
            }
        }

        assert(sceneByteSize == (size_t)(pDst - data.pBuffer));
    }
}