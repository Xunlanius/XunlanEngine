#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/Abstract/Mesh.h"

namespace Xunlan::DX12
{
    class DX12Submesh final : public Submesh
    {
    public:

        DX12Submesh(const Ref<DataBuffer>& vertexBuffer, const Ref<DataBuffer>& indexBuffer, PrimitiveType primitiveType, uint32 materialIndex)
            : Submesh(vertexBuffer, indexBuffer, primitiveType, materialIndex) {}

    public:

        static Ref<DX12Submesh> Create(const CRef<SubmeshRawData>& submeshRawData);
    };

    class DX12Mesh final : public Mesh
    {
    public:

        explicit DX12Mesh(uint32 numMaterials) : Mesh(numMaterials) {}

    public:

        static Ref<DX12Mesh> Create(const CRef<MeshRawData>& meshRawData);
    };
}