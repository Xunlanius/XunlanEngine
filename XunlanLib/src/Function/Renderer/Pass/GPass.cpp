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

        m_albedoWS = rhi.CreateRT(width, height, TextureFormat::R8G8B8A8_Unorm);
        m_positionWS = rhi.CreateRT(width, height, TextureFormat::R32G32B32A32_Float);
        m_normalWS = rhi.CreateRT(width, height, TextureFormat::R16G16B16A16_Snorm);
        m_depth = rhi.CreateDepthBuffer(width, height);

        m_gBuffer = rhi.CreateCBuffer(CBufferType::GBuffer, sizeof(CStruct::GBuffer));
    }

    void GPass::Render(Ref<RenderContext> context)
    {
        RHI& rhi = RHI::Instance();

        std::vector<CRef<RenderTarget>> rts = {
            m_albedoWS,
            m_positionWS,
            m_normalWS
        };

        rhi.SetRT(context, rts, m_depth);
        rhi.ClearRT(context, rts, m_depth);
        rhi.SetViewport(context, 0, 0, m_width, m_height);

        CollectRenderItems();
        RenderItems(context);

        rhi.ResetRT(context, rts, m_depth);

        CStruct::GBuffer* gBuffer = (CStruct::GBuffer*)m_gBuffer->GetData();
        gBuffer->m_albedoIndex = m_albedoWS->GetHeapIndex();
        gBuffer->m_positionIndex = m_positionWS->GetHeapIndex();
        gBuffer->m_normalIndex = m_normalWS->GetHeapIndex();

        m_gBuffer->Bind(context);
    }

    void GPass::CollectRenderItems()
    {
        m_renderItems.clear();

        WeakRef<Entity> refRoot = Scene::Instance().GetRoot();
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