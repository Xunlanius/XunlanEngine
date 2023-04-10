#include "MeshLightPass.h"
#include "src/Function/Renderer/RHI.h"
#include "src/Function/World/Scene.h"
#include "src/Function/World/Component/MeshRenderComponent.h"

namespace Xunlan
{
    MeshLightPass::MeshLightPass(uint32 width, uint32 height)
    {
        m_mainRT = RHI::Instance().CreateRenderTarget(width, height, RenderTargetUsage::DEFAULT);
    }

    void MeshLightPass::Render(const Ref<RenderContext>& context)
    {
        RHI& rhi = RHI::Instance();

        rhi.SetRenderTarget(context, m_mainRT);
        rhi.ClearRenderTarget(context, m_mainRT);
        rhi.SetViewport(context, 0, 0, m_mainRT->GetWidth(), m_mainRT->GetHeight());

        CollectRenderItems();

        for (const WeakRef<RenderItem>& refItem : m_renderItems)
        {
            Ref<RenderItem> item = refItem.lock();
            assert(item);

            item->Render(context);
        }

        rhi.ResetRenderTarget(context, m_mainRT);
    }

    void MeshLightPass::CollectRenderItems()
    {
        m_renderItems.clear();

        WeakRef<Entity> refRoot = Singleton<Scene>::Instance().GetRoot();
        CollectVisableEntity(refRoot);
    }
    void MeshLightPass::CollectVisableEntity(const WeakRef<Entity>& refNode)
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
}