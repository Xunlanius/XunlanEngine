#include "RenderPassBase.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "src/Function/Resource/ResourceType/MeshData.h"

namespace Xunlan
{
    void RenderPassBase::UpdateCBufferPerObject(const TransformerComponent& transformer, Ref<CBuffer> cBufferPerObject)
    {
        CStruct::PerObject* perObject = (CStruct::PerObject*)cBufferPerObject->GetData();
        perObject->m_world = TransformerSystem::GetWorld(transformer);
        perObject->m_invWorld = Math::GetInverse(perObject->m_world);
    }

    Ref<RenderItem> RenderPassBase::CreateCanvas()
    {
        RHI& rhi = RHI::Instance();

        const VertexData vertex0 = { { -1.0f, 1.0f, 0.0f }, {}, {}, { 0.0f, 0.0f } };
        const VertexData vertex1 = { { -1.0f, -3.0f, 0.0f }, {}, {}, { 0.0f, 2.0f } };
        const VertexData vertex2 = { { 3.0f, 1.0f, 0.0f }, {}, {}, { 2.0f, 0.0f } };

        VertexData vertexBuffer[3] = { vertex0, vertex1, vertex2 };
        uint32 indexBuffer[3] = { 0, 1, 2 };

        Ref<MeshRawData> meshRawData = MakeRef<MeshRawData>();
        Ref<SubmeshRawData>& submeshRawData = meshRawData->m_submeshRawDatas.emplace_back(MakeRef<SubmeshRawData>());

        RawData& vertexRawData = submeshRawData->m_vertexBuffer;
        RawData& indexRawData = submeshRawData->m_indexBuffer;

        vertexRawData.m_buffer = std::make_unique<byte[]>(sizeof(vertexBuffer));
        memcpy(vertexRawData.m_buffer.get(), vertexBuffer, sizeof(vertexBuffer));
        vertexRawData.m_numElements = _countof(vertexBuffer);
        vertexRawData.m_stride = sizeof(VertexData);

        indexRawData.m_buffer = std::make_unique<byte[]>(sizeof(indexBuffer));
        memcpy(indexRawData.m_buffer.get(), indexBuffer, sizeof(indexBuffer));
        indexRawData.m_numElements = _countof(indexBuffer);
        indexRawData.m_stride = sizeof(uint32);

        submeshRawData->primitiveType = PrimitiveType::TRIANGLE;

        Ref<Mesh> canvasMesh = rhi.CreateMesh(meshRawData);
        return rhi.CreateRenderItem(canvasMesh);
    }
}