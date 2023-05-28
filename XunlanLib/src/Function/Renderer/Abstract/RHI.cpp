#include "RHI.h"
#include "src/Function/Resource/AssetSystem.h"
#include "src/Function/Resource/ResourceType/MeshData.h"
#include "src/Function/Renderer/DX12/DX12RHI.h"

namespace Xunlan
{
    RHI& RHI::Create(Platform platform)
    {
        switch (platform)
        {
        case Platform::DX12: ms_instance = new DX12::DX12RHI(platform); break;
        default: assert(false);
        }

        return *ms_instance;
    }

    Ref<Mesh> RHI::CreateMesh(const std::filesystem::path& path)
    {
        MeshData meshData = {};
        AssetSystem::LoadAsset(path, meshData);

        Ref<MeshRawData> meshRawData = MakeRef<MeshRawData>();

        for (const SubmeshData& submeshData : meshData.submeshes)
        {
            Ref<SubmeshRawData>& submeshRawData = meshRawData->m_submeshRawDatas.emplace_back(MakeRef<SubmeshRawData>());

            RawData& vertexBuffer = submeshRawData->m_vertexBuffer;
            RawData& indexBuffer = submeshRawData->m_indexBuffer;

            vertexBuffer.m_numElements = (uint32)submeshData.vertexBuffer.size();
            vertexBuffer.m_stride = (uint32)sizeof(decltype(submeshData.vertexBuffer)::value_type);
            const uint32 vertexBufferSize = vertexBuffer.m_numElements * vertexBuffer.m_stride;
            vertexBuffer.m_buffer = std::make_unique<byte[]>(vertexBufferSize);
            memcpy(vertexBuffer.m_buffer.get(), submeshData.vertexBuffer.data(), vertexBufferSize);

            indexBuffer.m_numElements = (uint32)submeshData.indexBuffer.size();
            indexBuffer.m_stride = (uint32)sizeof(decltype(submeshData.indexBuffer)::value_type);
            const uint32 indexBufferSize = indexBuffer.m_numElements * indexBuffer.m_stride;
            indexBuffer.m_buffer = std::make_unique<byte[]>(indexBufferSize);
            memcpy(indexBuffer.m_buffer.get(), submeshData.indexBuffer.data(), indexBufferSize);

            submeshRawData->primitiveType = PrimitiveType::TRIANGLE;
            submeshRawData->materialIndex; // TODO:
        }

        meshRawData->m_numMaterials = 0; // TODO:

        return CreateMesh(meshRawData);
    }
    Ref<ImageTexture> RHI::CreateImageTexture(const std::filesystem::path& path)
    {
        throw;
    }
}