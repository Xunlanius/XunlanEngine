#include "DX12Mesh.h"
#include "DX12DataBuffer.h"

namespace Xunlan::DX12
{
    Ref<DX12Submesh> DX12Submesh::Create(CRef<SubmeshRawData> submeshRawData)
    {
        const Ref<DataBuffer> vertexBuffer = DX12DataBuffer::Create(submeshRawData->m_vertexBuffer);
        const Ref<DataBuffer> indexBuffer = DX12DataBuffer::Create(submeshRawData->m_indexBuffer);

        return MakeRef<DX12Submesh>(vertexBuffer, indexBuffer, submeshRawData->primitiveType, submeshRawData->materialIndex);
    }

    Ref<DX12Mesh> DX12Mesh::Create(CRef<MeshRawData> meshRawData)
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