#include "Geometry.h"
#include "src/Utility/IO.h"

namespace Xunlan::Tools
{
    namespace
    {
        using namespace Math;

        bool SplitMeshesByMaterial(uint32 materialID, const Mesh& mesh, Mesh& submesh)
        {
            submesh.name = mesh.name;
            submesh.lodThreshold = mesh.lodThreshold;
            submesh.lodID = mesh.lodID;
            submesh.materialIDs.push_back(materialID);
            submesh.uvSets.resize(mesh.uvSets.size());

            const uint32 numPolygons = (uint32)mesh.indices.size() / 3;
            std::vector<uint32> vertexRef(mesh.positions.size(), UINT32_MAX);

            for (uint32 polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
            {
                const uint32 materialIndex = mesh.materialIndices[polygonIndex];

                // if it's not the material we are going to process, just skip it
                if (materialIndex != materialID) continue;

                // Get the right material
                uint32 index = polygonIndex * 3;
                for (uint32 j = index; j < index + 3; ++j)
                {
                    const uint32 vertexIndex = mesh.indices[j];
                    if (vertexRef[vertexIndex] == UINT32_MAX) // the vertex hasn't encountered before
                    {
                        submesh.positions.push_back(mesh.positions[vertexIndex]);
                        submesh.indices.push_back((uint32)submesh.positions.size() - 1);
                        vertexRef[vertexIndex] = submesh.indices.back();
                    }
                    else
                    {
                        submesh.indices.push_back(vertexRef[vertexIndex]);
                    }

                    if (mesh.normals.size() > 0) submesh.normals.push_back(mesh.normals[j]);
                    if (mesh.tangents.size() > 0) submesh.tangents.push_back(mesh.tangents[j]);

                    const uint32 numUVSets = (uint32)mesh.uvSets.size();
                    for (uint32_t k = 0; k < numUVSets; ++k)
                    {
                        if (mesh.uvSets[k].size() > 0) submesh.uvSets[k].push_back(mesh.uvSets[k][j]);
                    }
                }
            }

            assert(submesh.indices.size() % 3 == 0);
            return submesh.indices.size() > 0;
        }
        void SplitMeshesByMaterial(Scene& scene)
        {
            for (LODGroup& lodGroup : scene.lodGroups)
            {
                std::vector<Mesh> oldMeshes;
                oldMeshes.swap(lodGroup.meshes);

                for (Mesh& mesh : oldMeshes)
                {
                    const uint32 numMaterials = (uint32)mesh.materialIDs.size();

                    if (numMaterials <= 1) // the mesh has a single material, don't need to split
                    {
                        lodGroup.meshes.emplace_back(mesh);
                        continue;
                    }
                    
                    for (uint32 i = 0; i < numMaterials; ++i)
                    {
                        Mesh& submesh = lodGroup.meshes.emplace_back();
                        if (!SplitMeshesByMaterial(mesh.materialIDs[i], mesh, submesh))
                        {
                            assert(false);
                            lodGroup.meshes.pop_back();
                        }
                    }
                }
            }
        }

        void RecalculateNormals(Mesh& mesh)
        {
            using namespace DirectX;

            const uint32 numIndices = (uint32)mesh.indices.size();
            assert(numIndices > 0);
            assert(numIndices % 3 == 0);

            mesh.normals.resize(numIndices);

            for (uint32 i = 0; i < numIndices;)
            {
                const uint32 index0 = mesh.indices[i];
                const uint32 index1 = mesh.indices[i + 1];
                const uint32 index2 = mesh.indices[i + 2];

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

        /// <summary>
        /// Construct and merge vertices (by normal)
        /// </summary>
        /// <param name="smoothingAngle: "> angle between faces </param>
        void MergeVerticesByNormal(Mesh& mesh, float smoothingAngle)
        {
            using namespace DirectX;

            // n: the angle between normals
            // cos(n) >= cosAlpha: soft edge
            // cos(n) < cosAlpha: hard edge
            assert(smoothingAngle >= 0.0f && smoothingAngle <= 180.0f);
            const float cosAlpha = XMScalarCos(XM_PI * (1 - smoothingAngle / 180.0f));
            const bool absolutelySoft = XMScalarNearEqual(smoothingAngle, 0.0f, EPSILON);
            const bool absolutelyHard = XMScalarNearEqual(smoothingAngle, 180.0f, EPSILON);

            std::vector<uint32> oldIndices(mesh.indices.size(), UINT_MAX);
            oldIndices.swap(mesh.indices);

            const uint32 numVertices = (uint32)mesh.positions.size();
            const uint32 numIndices = (uint32)oldIndices.size();
            assert(numVertices > 0 && numIndices > 0 && numIndices % 3 == 0);

            //indicesOfVertices[i]: indices of the indices array that holds by vertex i
            std::vector<std::vector<uint32>> indicesOfVertices(numVertices);

            for (uint32 i = 0; i < numIndices; ++i)
            {
                indicesOfVertices[oldIndices[i]].push_back(i);
            }

            for (const auto& indicesOfVertex : indicesOfVertices)
            {
                const uint32 numIndicesOfVertex = (uint32)indicesOfVertex.size();

                for (uint32 j = 0; j < numIndicesOfVertex; ++j)
                {
                    const uint32 index = indicesOfVertex[j];

                    // if the index has been encountered before, just skip it
                    if (mesh.indices[index] != UINT_MAX) continue;

                    // Add a vertex
                    Vertex& vertex = mesh.vertices.emplace_back();
                    mesh.indices[index] = (uint32)mesh.vertices.size() - 1;

                    // Store the position of the vertex
                    vertex.position = mesh.positions[oldIndices[index]];

                    if (absolutelyHard) // Hard edge
                    {
                        vertex.normal = mesh.normals[index];
                        continue;
                    }

                    // Calculate the merged normal of the vertex
                    XMVECTOR normal1 = XMLoadFloat3(&mesh.normals[index]);

                    for (uint32 k = j + 1; k < numIndicesOfVertex; ++k)
                    {
                        const uint32 otherIndex = indicesOfVertex[k];

                        // if the index has been encountered before, just skip it
                        if (mesh.indices[otherIndex] != UINT_MAX) continue;

                        XMVECTOR normal2 = XMLoadFloat3(&mesh.normals[otherIndex]);

                        // cosTheta of the angle between normals
                        // cos(angle) = dot(normal1, normal2) / (||normal1|| * ||normal2||)
                        float cosTheta = 0;
                        if (!absolutelySoft) XMStoreFloat(&cosTheta, XMVector3Dot(normal1, normal2));

                        if (absolutelySoft || cosTheta >= cosAlpha) // The edge is soft, merge the vertex
                        {
                            normal1 = XMVector3Normalize(normal1 + normal2);
                            mesh.indices[otherIndex] = mesh.indices[index];
                        }
                    }
                    XMStoreFloat3(&vertex.normal, normal1);
                }
            }
        }

        /// <summary>
        /// Construct the vertices (merged by uv)
        /// </summary>
        void MergeVerticesByUVSets(Mesh& mesh)
        {
            using namespace DirectX;

            std::vector<Vertex> oldVertices;
            oldVertices.swap(mesh.vertices);
            std::vector<uint32> oldIndices(mesh.indices.size(), UINT_MAX);
            oldIndices.swap(mesh.indices);

            const uint32 numVertices = (uint32)oldVertices.size();
            const uint32 numIndices = (uint32)oldIndices.size();
            assert(numVertices > 0 && numIndices > 0 && numIndices % 3 == 0);

            std::vector<std::vector<uint32>> indicesOfVertices(numVertices);

            for (uint32 i = 0; i < numIndices; ++i)
            {
                indicesOfVertices[oldIndices[i]].push_back(i);
            }

            for (const auto& indicesOfVertex : indicesOfVertices)
            {
                uint32 numIndicesOfVertices = (uint32)indicesOfVertex.size();

                for (uint32 j = 0; j < numIndicesOfVertices; ++j)
                {
                    const uint32 index = indicesOfVertex[j];

                    // if the index has been encountered before, just skip it
                    if (mesh.indices[index] != UINT_MAX) continue;

                    // Add a vertex
                    Vertex& vertex = mesh.vertices.emplace_back(oldVertices[oldIndices[index]]);
                    mesh.indices[index] = (uint32)mesh.vertices.size() - 1;

                    vertex.uv = mesh.uvSets[0][index];

                    for (uint32 k = j + 1; k < numIndicesOfVertices; ++k)
                    {
                        const uint32 otherIndex = indicesOfVertex[k];

                        // if the index has been encountered before, just skip it
                        if (mesh.indices[otherIndex] != UINT_MAX) continue;

                        const UV& uv = mesh.uvSets[0][otherIndex];

                        // If two uvs are nearly equal, merge it
                        if (XMScalarNearEqual(vertex.uv.x, uv.x, EPSILON) && XMScalarNearEqual(vertex.uv.y, uv.y, EPSILON))
                        {
                            mesh.indices[otherIndex] = mesh.indices[index];
                        }
                    }
                }
            }
        }

        void DetermineElementsType(Mesh& mesh)
        {
            using namespace Element;

            mesh.elementsType = {};

            if (!mesh.normals.empty())
            {
                if (!mesh.uvSets.empty() && !mesh.uvSets[0].empty()) mesh.elementsType = ElementType::NORMAL_TEXTURE;
                else mesh.elementsType = ElementType::NORMAL;
            }
            else if (!mesh.colors.empty())
            {
                mesh.elementsType = ElementType::COLOR;
            }

            // TODO: Skeletal
        }

        uint64 GetElementSize(Element::ElementType::Type type)
        {
            using namespace Element;

            if (type == ElementType::Type::NORMAL) return sizeof(Normal);
            else if (type == ElementType::Type::NORMAL_TEXTURE) return sizeof(NormalTexture);
            else if (type == ElementType::Type::COLOR) return sizeof(Color);
            else if (type == ElementType::Type::SKELETAL) return sizeof(Skeletal);
            else if (type == ElementType::Type::SKELETAL_NORMAL) return sizeof(SkeletalNormal);
            else if (type == ElementType::Type::SKELETAL_COLOR) return sizeof(SkeletalColor);
            else if (type == ElementType::Type::SKELETAL_NORMAL_TEXTURE) return sizeof(SkeletalNormalTexture);
            else if (type == ElementType::Type::SKELETAL_NORMAL_COLOR) return sizeof(SkeletalNormalColor);
            else if (type == ElementType::Type::SKELETAL_NORMAL_TEXTURE_COLOR) return sizeof(SkeletalNormalTextureColor);

            return 0;
        }

        /// <summary>
        /// Pack vertices for saving space
        /// </summary>
        void PackVertices(Mesh& mesh)
        {
            using namespace Element;

            const ElementType::Type type = mesh.elementsType;
            const uint32 numVertices = (uint32)mesh.vertices.size();
            assert(numVertices > 0);

            mesh.positionBuffer.resize(numVertices * sizeof(Math::float3));
            mesh.elementBuffer.resize(numVertices * GetElementSize(type));
            Math::float3* const positionBuffer = (Math::float3*)mesh.positionBuffer.data();

            for (uint32 i = 0; i < numVertices; ++i)
            {
                positionBuffer[i] = mesh.vertices[i].position;
            }

            std::vector<byte> signs(numVertices);
            std::vector<std::pair<uint16, uint16>> normals(numVertices);
            std::vector<std::pair<uint16, uint16>> tangents(numVertices);
            std::vector<std::tuple<byte, byte, byte>> jointWeights(numVertices);

            // Normals
            if (type & ElementType::NORMAL)
            {
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    signs[i] = (byte)(vertex.normal.z > 0.0f) << 1;
                    normals[i] = {
                        (uint16)PackFloat<16>(vertex.normal.x, -1.0f, 1.0f),
                        (uint16)PackFloat<16>(vertex.normal.y, -1.0f, 1.0f)
                    };
                }

                if (type & ElementType::NORMAL_TEXTURE)
                {
                    // Full T-space
                    for (uint32 i = 0; i < numVertices; ++i)
                    {
                        Vertex& vertex = mesh.vertices[i];
                        signs[i] |= (byte)((vertex.tangent.w > 0.0f) && (vertex.tangent.z > 0.0f));
                        tangents[i] = {
                            (uint16)PackFloat<16>(vertex.tangent.x, -1.0f, 1.0f),
                            (uint16)PackFloat<16>(vertex.tangent.y, -1.0f, 1.0f)
                        };
                    }
                }
            }

            // Skeletal
            if (type & ElementType::SKELETAL)
            {
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    jointWeights[i] = {
                        (byte)PackUnitFloat<8>(vertex.jointWeights.x),
                        (byte)PackUnitFloat<8>(vertex.jointWeights.y),
                        (byte)PackUnitFloat<8>(vertex.jointWeights.z),
                    };
                }
            }

            if (type == ElementType::NORMAL)
            {
                Normal* const elementBuffer = (Normal*)mesh.elementBuffer.data();
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    elementBuffer[i] = {
                        { vertex.red, vertex.green, vertex.blue },
                        signs[i],
                        { normals[i].first, normals[i].second },
                    };
                }
            }
            else if (type == ElementType::NORMAL_TEXTURE)
            {
                NormalTexture* const elementBuffer = (NormalTexture*)mesh.elementBuffer.data();
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    elementBuffer[i] = {
                        { vertex.red, vertex.green, vertex.blue },
                        signs[i],
                        { normals[i].first, normals[i].second },
                        { tangents[i].first, tangents[i].second },
                        vertex.uv,
                    };
                }
            }
            else if (type == ElementType::COLOR)
            {
                Color* const elementBuffer = (Color*)mesh.elementBuffer.data();
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    elementBuffer[i] = { { vertex.red, vertex.green, vertex.blue }, {} };
                }
            }
            else if (type == ElementType::SKELETAL)
            {
                Skeletal* const elementBuffer = (Skeletal*)mesh.elementBuffer.data();
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    elementBuffer[i] = {
                        { std::get<0>(jointWeights[i]), std::get<1>(jointWeights[i]), std::get<2>(jointWeights[i]) },
                        {},
                        { (uint16)vertex.jointIndices.x, (uint16)vertex.jointIndices.y, (uint16)vertex.jointIndices.z, (uint16)vertex.jointIndices.w },
                    };
                }
            }
            else if (type == ElementType::SKELETAL_NORMAL)
            {
                SkeletalNormal* const elementBuffer = (SkeletalNormal*)mesh.elementBuffer.data();
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    elementBuffer[i] = {
                        { std::get<0>(jointWeights[i]), std::get<1>(jointWeights[i]), std::get<2>(jointWeights[i]) },
                        signs[i],
                        { (uint16)vertex.jointIndices.x, (uint16)vertex.jointIndices.y, (uint16)vertex.jointIndices.z, (uint16)vertex.jointIndices.w },
                        { normals[i].first, normals[i].second },
                    };
                }
            }
            else if (type == ElementType::SKELETAL_COLOR)
            {
                SkeletalColor* const elementBuffer = (SkeletalColor*)mesh.elementBuffer.data();
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    elementBuffer[i] = {
                        { std::get<0>(jointWeights[i]), std::get<1>(jointWeights[i]), std::get<2>(jointWeights[i]) },
                        {},
                        { (uint16)vertex.jointIndices.x, (uint16)vertex.jointIndices.y, (uint16)vertex.jointIndices.z, (uint16)vertex.jointIndices.w },
                        { vertex.red, vertex.green, vertex.blue },
                        {},
                    };
                }
            }
            else if (type == ElementType::SKELETAL_NORMAL_TEXTURE)
            {
                SkeletalNormalTexture* const elementBuffer = (SkeletalNormalTexture*)mesh.elementBuffer.data();
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    elementBuffer[i] = {
                        { std::get<0>(jointWeights[i]), std::get<1>(jointWeights[i]), std::get<2>(jointWeights[i]) },
                        signs[i],
                        { (uint16)vertex.jointIndices.x, (uint16)vertex.jointIndices.y, (uint16)vertex.jointIndices.z, (uint16)vertex.jointIndices.w },
                        { normals[i].first, normals[i].second },
                        { tangents[i].first, tangents[i].second },
                        vertex.uv,
                    };
                }
            }
            else if (type == ElementType::SKELETAL_NORMAL_COLOR)
            {
                SkeletalNormalColor* const elementBuffer = (SkeletalNormalColor*)mesh.elementBuffer.data();
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    elementBuffer[i] = {
                        { std::get<0>(jointWeights[i]), std::get<1>(jointWeights[i]), std::get<2>(jointWeights[i]) },
                        signs[i],
                        { (uint16)vertex.jointIndices.x, (uint16)vertex.jointIndices.y, (uint16)vertex.jointIndices.z, (uint16)vertex.jointIndices.w },
                        { normals[i].first, normals[i].second },
                        { vertex.red, vertex.green, vertex.blue },
                        {},
                    };
                }
            }
            else if (type == ElementType::SKELETAL_NORMAL_TEXTURE_COLOR)
            {
                SkeletalNormalTextureColor* const elementBuffer = (SkeletalNormalTextureColor*)mesh.elementBuffer.data();
                for (uint32 i = 0; i < numVertices; ++i)
                {
                    Vertex& vertex = mesh.vertices[i];
                    elementBuffer[i] = {
                        { std::get<0>(jointWeights[i]), std::get<1>(jointWeights[i]), std::get<2>(jointWeights[i]) },
                        signs[i],
                        { (uint16)vertex.jointIndices.x, (uint16)vertex.jointIndices.y, (uint16)vertex.jointIndices.z, (uint16)vertex.jointIndices.w },
                        { normals[i].first, normals[i].second },
                        { tangents[i].first, tangents[i].second },
                        vertex.uv,
                        { vertex.red, vertex.green, vertex.blue },
                        {},
                    };
                }
            }
        }

        void ProcessVertices(Mesh& mesh, const GeometryImportSettings& settings)
        {
            assert(mesh.indices.size() > 0);
            assert(mesh.indices.size() % 3 == 0);

            if (settings.calculateNormals || mesh.normals.empty()) RecalculateNormals(mesh);

            MergeVerticesByNormal(mesh, settings.smoothingAngle);

            if (!mesh.uvSets.empty()) MergeVerticesByUVSets(mesh);

            DetermineElementsType(mesh);
            PackVertices(mesh);
        }

        size_t ComputeMeshSize(const Mesh& mesh)
        {
            const size_t nameBufferSize = mesh.name.size();
            const size_t positionBufferSize = mesh.positionBuffer.size();
            const size_t elementBufferSize = mesh.elementBuffer.size();
            const size_t indexBufferSize = sizeof(uint32) * mesh.indices.size();

            const size_t byteSize = {
                sizeof(uint32) +        // Name length
                nameBufferSize +        // Name
                sizeof(uint32) +        // Number of vertices
                sizeof(uint32) +        // Number of indices
                sizeof(uint32) +        // Element type
                sizeof(uint32) +        // Element size
                sizeof(uint32) +        // LOD ID
                sizeof(float) +         // LOD threshold
                positionBufferSize +    // Position buffer size
                indexBufferSize +       // Index buffer size
                elementBufferSize       // Element buffer size
            };

            return byteSize;
        }
        size_t ComputeSceneSize(const Scene& scene)
        {
            size_t size = sizeof(uint32) + scene.name.size() + sizeof(uint32);

            for (const LODGroup& lodGroup : scene.lodGroups)
            {
                size += sizeof(uint32) + lodGroup.name.size() + sizeof(uint32);

                for (const Mesh& mesh : lodGroup.meshes)
                {
                    size += ComputeMeshSize(mesh);
                }
            }

            return size;
        }

        void PackMesh(const Mesh& mesh, byte*& dst)
        {
            // Mesh name
            uint32 nameLen = (uint32)mesh.name.size();
            IO::Write<uint32>(nameLen, dst);
            IO::WriteBuffer(dst, mesh.name.data(), nameLen);

            // Number of vertices
            IO::Write<uint32>((uint32)mesh.vertices.size(), dst);

            // Number of indices
            uint32 numIndices = (uint32)mesh.indices.size();
            IO::Write<uint32>(numIndices, dst);

            // Element type
            IO::Write<uint32>(mesh.elementsType, dst);
            // Element size
            IO::Write<uint32>((uint32)GetElementSize(mesh.elementsType), dst);

            // LOD ID
            IO::Write<uint32>(mesh.lodID, dst);
            // LOD threshold
            IO::Write<float>(mesh.lodThreshold, dst);

            // Position buffer
            IO::WriteBuffer(dst, mesh.positionBuffer.data(), mesh.positionBuffer.size());
            // Index buffer
            IO::WriteBuffer(dst, mesh.indices.data(), numIndices * sizeof(uint32));
            // Element buffer
            IO::WriteBuffer(dst, mesh.elementBuffer.data(), mesh.elementBuffer.size());
        }
    }

    void ProcessScene(Scene& scene, const GeometryImportSettings& settings)
    {
        SplitMeshesByMaterial(scene);

        for (LODGroup& lodGroup : scene.lodGroups)
        {
            for (Mesh& mesh : lodGroup.meshes)
            {
                ProcessVertices(mesh, settings);
            }
        }
    }

    void Pack(const Scene& scene, SceneData& data)
    {
        const size_t sceneByteSize = ComputeSceneSize(scene);
        assert(sceneByteSize > 0);

        data.bufferByteSize = (uint32)sceneByteSize;
        data.pBuffer = (byte*)CoTaskMemAlloc(sceneByteSize);
        assert(data.pBuffer);

        byte* dst = data.pBuffer;

        // Scene name
        uint32 sceneNameLen = (uint32)scene.name.size();
        IO::Write<uint32>(sceneNameLen, dst);
        IO::WriteBuffer(dst, scene.name.data(), sceneNameLen);

        // Number of LODs
        IO::Write<uint32>((uint32)scene.lodGroups.size(), dst);

        for (const LODGroup& lodGroup : scene.lodGroups)
        {
            // LOD name
            uint32 lodNameLen = (uint32)lodGroup.name.size();
            IO::Write<uint32>(lodNameLen, dst);
            IO::WriteBuffer(dst, lodGroup.name.data(), lodNameLen);

            // Number of Meshes
            IO::Write<uint32>((uint32)lodGroup.meshes.size(), dst);

            // Pack meshes
            for (const Mesh& mesh : lodGroup.meshes)
            {
                PackMesh(mesh, dst);
            }
        }

        assert(dst == data.pBuffer + sceneByteSize);
    }
}