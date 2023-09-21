#include "DX12RenderItem.h"
#include "DX12RenderContext.h"
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
            Ref<DX12DataBuffer> verticesView = CastTo<DX12DataBuffer>(submesh->GetVerticesView());
            Ref<DX12DataBuffer> indicesView = CastTo<DX12DataBuffer>(submesh->GetIndicesView());

            context->SetParam("g_vertices", verticesView);

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

            D3D12_INDEX_BUFFER_VIEW dx12IndicesView = {};
            dx12IndicesView.BufferLocation = indicesView->GetGPUAddress();
            dx12IndicesView.SizeInBytes = indicesView->GetNumElement() * indicesView->GetStride();
            dx12IndicesView.Format = DXGI_FORMAT_R32_UINT;

            cmdList->IASetIndexBuffer(&dx12IndicesView);
            cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            cmdList->DrawIndexedInstanced(indicesView->GetNumElement(), 1, indicesView->GetOffset(), verticesView->GetOffset(), 0);
        }
    }
}