#include "GPass.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "src/Function/World/Scene.h"
#include "src/Function/World/Component/MeshRender.h"

namespace Xunlan
{
    GPass::GPass(uint32 width, uint32 height)
        : m_width(width), m_height(height)
    {
        RHI& rhi = RHI::Instance();

        m_albedo = rhi.CreateRT(width, height);
        m_position = rhi.CreateRT(width, height);
        m_normal = rhi.CreateRT(width, height);
        m_depthBuffer = rhi.CreateDepthBuffer(width, height);

        m_gBuffer = rhi.CreateCBuffer(CBufferType::GBuffer, sizeof(CStruct::GBuffer));
    }

    void GPass::Render(Ref<RenderContext> context)
    {
        RHI& rhi = RHI::Instance();

        std::vector<CRef<RenderTarget>> rts = {
            m_albedo,
            m_position,
            m_normal
        };

        rhi.SetRT(context, rts, m_depthBuffer);
        rhi.ClearRT(context, rts, m_depthBuffer);
        rhi.SetViewport(context, 0, 0, m_width, m_height);

        CollectRenderItems();
        RenderItems(context);

        rhi.ResetRT(context, rts, m_depthBuffer);

        CStruct::GBuffer* gBuffer = (CStruct::GBuffer*)m_gBuffer->GetData();
        gBuffer->m_albedoIndex = m_albedo->GetHeapIndex();
        gBuffer->m_positionIndex = m_position->GetHeapIndex();
        gBuffer->m_normalIndex = m_normal->GetHeapIndex();

        m_gBuffer->Bind(context);
    }

    void GPass::CollectRenderItems()
    {
        m_renderItems.clear();

        WeakRef<Entity> refRoot = Singleton<Scene>::Instance().GetRoot();
        CollectVisableEntity(refRoot);
    }

    void GPass::CollectVisableEntity(const WeakRef<Entity>& refNode)
    {
        Ref<Entity> node = refNode.lock();
        assert(node);

        if (node->HasComponent<MeshRenderComponent>())
        {
            auto [transformer, meshRender] = node->GetComponent<TransformerComponent, MeshRenderComponent>();

            UpdateCBufferPerObject(transformer, meshRender.m_renderItem->GetPerObject());
            m_renderItems.push_back(meshRender.m_renderItem);
        }

        for (const WeakRef<Entity>& child : node->GetChildren())
        {
            CollectVisableEntity(child);
        }
    }

    void GPass::RenderItems(Ref<RenderContext> context)
    {
        for (const WeakRef<RenderItem>& refItem : m_renderItems)
        {
            Ref<RenderItem> item = refItem.lock();
            assert(item);

            item->Render(context);
        }
    }
}