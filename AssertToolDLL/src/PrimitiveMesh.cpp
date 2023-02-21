#include "PrimitiveMesh.h"

namespace Xunlan::Tools
{
    namespace
    {
        using namespace Math;

        using MeshCreator = void (*)(Scene&, const PrimitiveMeshInitInfo&);

        void CreatePlane(Scene& scene, const PrimitiveMeshInitInfo& info);
        void CreateCube(Scene& scene, const PrimitiveMeshInitInfo& info);
        void CreateUVSphere(Scene& scene, const PrimitiveMeshInitInfo& info);
        void CreateIcoSphere(Scene& scene, const PrimitiveMeshInitInfo& info);
        void CreateCylinder(Scene& scene, const PrimitiveMeshInitInfo& info);
        void CreateCapsule(Scene& scene, const PrimitiveMeshInitInfo& info);

        MeshCreator g_meshCreators[] = {
            CreatePlane,
            CreateCube,
            CreateUVSphere,
            CreateIcoSphere,
            CreateCylinder,
            CreateCapsule,
        };

        static_assert(_countof(g_meshCreators) == (int)PrimitiveMeshType::Count);

        struct Axis
        {
            enum : uint32
            {
                X = 0,
                Y = 1,
                Z = 2,

                Count,
            };
        };

        Mesh InternalCreatePlane(
            const PrimitiveMeshInitInfo& info,
            uint32 horizontalIndex = Axis::X, uint32 verticalIndex = Axis::Z, bool flipWinding = false,
            Vector3 offset = { -0.5f, 0.0f, -0.5f }, Vector2 uRange = { 0.0f, 1.0f }, Vector2 vRange = { 0.0f, 1.0f })
        {
            assert(horizontalIndex < Axis::Count);
            assert(verticalIndex < Axis::Count);
            assert(horizontalIndex != verticalIndex);

            const uint32 horizontalCount = Clamp(info.segments[horizontalIndex], 1u, 100u);
            const uint32 verticalCount = Clamp(info.segments[verticalIndex], 1u, 100u);
            const float horizontalStride = 1.0f / horizontalCount;
            const float verticalStride = 1.0f / verticalCount;
            const float uStride = (uRange.y - uRange.x) / horizontalCount;
            const float vStride = (vRange.y - vRange.x) / verticalCount;

            Mesh mesh = {};
            std::vector<UV> uvs;

            // Prepare for positions and uvs
            for (uint32 i = 0; i <= verticalCount; ++i)
            {
                for (uint32 j = 0; j <= horizontalCount; ++j)
                {
                    Vector3 position = offset;
                    float* const asArray = &position.x;
                    asArray[horizontalIndex] += j * horizontalStride;
                    asArray[verticalIndex] += i * verticalStride;

                    position.x *= info.size.x;
                    position.y *= info.size.y;
                    position.z *= info.size.z;

                    mesh.positions.push_back(position);

                    UV uv = { uRange.x + j * uStride, 1.0f - (vRange.x + i * vStride) };
                    uvs.push_back(uv);
                }
            }
            assert((uint32)mesh.positions.size() == (horizontalCount + 1) * (verticalCount + 1));

            // Prepare for rawIndices
            const uint32 numVerticesInRow = horizontalCount + 1;
            for (uint32 i = 0; i < verticalCount; ++i)
            {
                for (uint32 j = 0; j < horizontalCount; ++j)
                {
                    const uint32 index[4] =
                    {
                        i * numVerticesInRow + j,
                        (i + 1) * numVerticesInRow + j,
                        i * numVerticesInRow + (j + 1),
                        (i + 1) * numVerticesInRow + (j + 1),
                    };

                    mesh.rawIndices.emplace_back(index[0]);
                    mesh.rawIndices.emplace_back(index[flipWinding ? 2 : 1]);
                    mesh.rawIndices.emplace_back(index[flipWinding ? 1 : 2]);

                    mesh.rawIndices.emplace_back(index[2]);
                    mesh.rawIndices.emplace_back(index[flipWinding ? 3 : 1]);
                    mesh.rawIndices.emplace_back(index[flipWinding ? 1 : 3]);
                }
            }

            const uint32 numIndices = { 3 * 2 * horizontalCount * verticalCount };
            assert(mesh.rawIndices.size() == numIndices);

            UVSet& uvSet = mesh.uvSets.emplace_back();
            for (uint32 i = 0; i < numIndices; ++i)
            {
                uvSet.emplace_back(uvs[mesh.rawIndices[i]]);
            }

            return mesh;
        }

        void CreatePlane(Scene& scene, const PrimitiveMeshInitInfo& info)
        {
            LodGroup lodGroup = {};
            lodGroup.name = "Plane";
            lodGroup.meshes.push_back(InternalCreatePlane(info));

            scene.lodGroups.push_back(lodGroup);
        }
        void CreateCube(Scene& scene, const PrimitiveMeshInitInfo& info)
        {
        }
        void CreateUVSphere(Scene& scene, const PrimitiveMeshInitInfo& info)
        {
        }
        void CreateIcoSphere(Scene& scene, const PrimitiveMeshInitInfo& info)
        {
        }
        void CreateCylinder(Scene& scene, const PrimitiveMeshInitInfo& info)
        {
        }
        void CreateCapsule(Scene& scene, const PrimitiveMeshInitInfo& info)
        {
        }
    }

    EDITOR_INTERFACE void CreatePrimitiveMesh(SceneData* pData, PrimitiveMeshInitInfo* pInfo)
    {
        assert(pData);
        assert(pInfo);
        assert((int)pInfo->meshType >= 0);
        assert((int)pInfo->meshType < (int)PrimitiveMeshType::Count);
        if (!pData || !pInfo || (int)pInfo->meshType < 0 || (int)pInfo->meshType >= (int)PrimitiveMeshType::Count) return;

        // Create the scene
        Scene scene = {};
        g_meshCreators[(int)pInfo->meshType](scene, *pInfo);

        // Process and pack
        pData->settings.calculateNormals = 1;
        ProcessScene(scene, pData->settings);
        Pack(scene, *pData);
    }
}