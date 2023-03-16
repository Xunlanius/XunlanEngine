#include "PrimitiveMesh.h"

namespace Xunlan::Tools
{
    namespace
    {
        using namespace Math;

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

        Mesh CreatePlaneInternal(
            const PrimitiveMeshInitInfo& info,
            uint32 horizontalIndex = Axis::X,
            uint32 verticalIndex = Axis::Z,
            bool flipWinding = false,
            Vector3 offset = { -0.5f, 0.0f, -0.5f },
            Vector2 uRange = { 0.0f, 1.0f },
            Vector2 vRange = { 0.0f, 1.0f })
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
            assert(mesh.positions.size() == (horizontalCount + 1) * (verticalCount + 1));

            // Prepare for indices
            const uint32 numVerticesInRow = horizontalCount + 1;
            for (uint32 i = 0; i < verticalCount; ++i)
            {
                for (uint32 j = 0; j < horizontalCount; ++j)
                {
                    const uint32 index[4] =
                    {
                        numVerticesInRow * i + j,
                        numVerticesInRow * (i + 1) + j,
                        numVerticesInRow * i + (j + 1),
                        numVerticesInRow * (i + 1) + (j + 1),
                    };

                    mesh.indices.emplace_back(index[0]);
                    mesh.indices.emplace_back(index[flipWinding ? 2 : 1]);
                    mesh.indices.emplace_back(index[flipWinding ? 1 : 2]);

                    mesh.indices.emplace_back(index[2]);
                    mesh.indices.emplace_back(index[flipWinding ? 3 : 1]);
                    mesh.indices.emplace_back(index[flipWinding ? 1 : 3]);
                }
            }

            const uint32 numIndices = (uint32)mesh.indices.size();
            assert(numIndices == 3 * 2 * horizontalCount * verticalCount);

            UVSet& uvSet = mesh.uvSets.emplace_back();
            for (uint32 i = 0; i < numIndices; ++i)
            {
                uvSet.emplace_back(uvs[mesh.indices[i]]);
            }

            return mesh;
        }

        void CreatePlane(Scene& scene, const PrimitiveMeshInitInfo& info)
        {
            LODGroup& lodGroup = scene.lodGroups.emplace_back();
            lodGroup.name = "Plane";
            lodGroup.meshes.push_back(CreatePlaneInternal(info));
        }
        void CreateCube(Scene& scene, const PrimitiveMeshInitInfo& info) {}
        void CreateUVSphere(Scene& scene, const PrimitiveMeshInitInfo& info) {}
        void CreateIcoSphere(Scene& scene, const PrimitiveMeshInitInfo& info) {}
        void CreateCylinder(Scene& scene, const PrimitiveMeshInitInfo& info) {}
        void CreateCapsule(Scene& scene, const PrimitiveMeshInitInfo& info) {}

        using MeshCreator = void (*)(Scene&, const PrimitiveMeshInitInfo&);

        MeshCreator g_meshCreators[] =
        {
            CreatePlane,
            CreateCube,
            CreateUVSphere,
            CreateIcoSphere,
            CreateCylinder,
            CreateCapsule,
        };

        static_assert(_countof(g_meshCreators) == (uint32)PrimitiveMeshType::Count);
    }

    EDITOR_INTERFACE void CreatePrimitiveMesh(PrimitiveMeshInitInfo* initInfo, SceneData* sceneData)
    {
        if (!sceneData || !initInfo || (uint32)initInfo->meshType >= (uint32)PrimitiveMeshType::Count) { assert(false); return; };

        // Create the scene
        Scene scene = {};
        g_meshCreators[(uint32)initInfo->meshType](scene, *initInfo);

        sceneData->settings.calculateNormals = true;

        // Process and pack
        ProcessScene(scene, sceneData->settings);
        Pack(scene, *sceneData);
    }
}