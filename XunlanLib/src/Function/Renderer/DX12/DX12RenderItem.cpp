#include "DX12RenderItem.h"
#include "DX12RenderContext.h"
#include "DX12RootParameter.h"
#include "DX12DataBuffer.h"
#include "DX12RHI.h"

namespace Xunlan::DX12
{
    void DX12RenderItem::Render(Ref<RenderContext> context) const
    {
        if (m_materials.empty() || !GetMaterial(0)) { assert(false); return; }

        DrawMesh(context, nullptr);
    }
    void DX12RenderItem::Render(Ref<RenderContext> context, CRef<Material> overrideMaterial) const
    {
        DrawMesh(context, overrideMaterial);
    }

    void DX12RenderItem::DrawMesh(Ref<RenderContext> context, CRef<Material> overrideMaterial) const
    {
        DX12RHI& rhi = DX12RHI::Instance();
        Ref<DX12RenderContext> dx12Context = CastTo<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        const std::vector<Ref<Submesh>> submeshes = m_mesh->GetSubmeshes();

        for (const Ref<Submesh>& submesh : submeshes)
        {
            if (overrideMaterial)
            {
                overrideMaterial->Apply(context);
            }
            else
            {
                const Ref<Material> material = GetMaterial(submesh->GetMaterialIndex());
                assert(material);

                material->Apply(context);
            }

            m_perObject->Bind(context);

            Ref<DX12DataBuffer> verticesView = CastTo<DX12DataBuffer>(submesh->GetVerticesView());
            Ref<DX12DataBuffer> indicesView = CastTo<DX12DataBuffer>(submesh->GetIndicesView());

            /*D3D12_VERTEX_BUFFER_VIEW dx12VerticesView = {};
            dx12VerticesView.BufferLocation = verticesView->GetGPUAddress();
            dx12VerticesView.SizeInBytes = verticesView->GetNumElement() * verticesView->GetStride();
            dx12VerticesView.StrideInBytes = verticesView->GetStride();*/

            D3D12_INDEX_BUFFER_VIEW dx12IndicesView = {};
            dx12IndicesView.BufferLocation = indicesView->GetGPUAddress();
            dx12IndicesView.SizeInBytes = indicesView->GetNumElement() * indicesView->GetStride();
            dx12IndicesView.Format = DXGI_FORMAT_R32_UINT;

            //cmdList->IASetVertexBuffers(0, 1, &dx12VerticesView);
            cmdList->SetGraphicsRootShaderResourceView((uint32)RootParam::VertexBuffer, verticesView->GetGPUAddress());
            cmdList->IASetIndexBuffer(&dx12IndicesView);
            cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            cmdList->DrawIndexedInstanced(indicesView->GetNumElement(), 1, indicesView->GetOffset(), verticesView->GetOffset(), 0);
        }
    }
}