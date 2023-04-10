#include "DX12Mesh.h"
#include "DX12RenderBuffer.h"

namespace Xunlan::DX12
{
    Ref<DX12Submesh> DX12Submesh::Create(const CRef<SubmeshRawData>& submeshRawData)
    {
        const Ref<RenderBuffer> vertexBuffer = DX12RenderBuffer::Create(submeshRawData->m_vertexBuffer);
        const Ref<RenderBuffer> indexBuffer = DX12RenderBuffer::Create(submeshRawData->m_indexBuffer);

        return MakeRef<DX12Submesh>(vertexBuffer, indexBuffer, submeshRawData->primitiveType, submeshRawData->materialIndex);
    }

    Ref<DX12Mesh> DX12Mesh::Create(const CRef<MeshRawData>& meshRawData)
    {
        const Ref<DX12Mesh> mesh = MakeRef<DX12Mesh>(meshRawData->m_numMaterials);

        for (const CRef<SubmeshRawData>& submeshRawData : meshRawData->m_submeshRawDatas)
        {
            const Ref<DX12Submesh> submeshView = DX12Submesh::Create(submeshRawData);
            mesh->AddSubmesh(submeshView);
        }

        return mesh;
    }
}