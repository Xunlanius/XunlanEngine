#include "ShadowPass.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Function/World/Scene.h"
#include "src/Function/World/Component/MeshRender.h"

namespace Xunlan
{
    ShadowPass::ShadowPass()
    {
        RHI& rhi = RHI::Instance();

        m_shadowMap = rhi.CreateDepthBuffer(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
        m_shadowMapIndices = rhi.CreateCBuffer(CBufferType::ShadowMaps, sizeof(CBufferShadowMaps));

        ConfigSystem& configSystem = Singleton<ConfigSystem>::Instance();
        const std::filesystem::path shadowMappingShader = configSystem.GetHLSLFolder() / "ShadowMapping.hlsl";

        ShaderList list = {};
        list.m_VS = rhi.CreateShader(ShaderType::VERTEX_SHADER, shadowMappingShader, "VS");

        m_shadowMaterial = rhi.CreateMaterial("Shadow_Map_Material", MaterialType::ShadowMapping, list);
    }

    void ShadowPass::Render(Ref<RenderContext> context)
    {
        RHI& rhi = RHI::Instance();

        rhi.SetRT(context, m_shadowMap);
        rhi.ClearRT(context, m_shadowMap);
        rhi.SetViewport(context, 0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

        CollectRenderItems();

        for (const WeakRef<RenderItem>& refItem : m_renderItems)
        {
            Ref<RenderItem> item = refItem.lock();
            assert(item);
            item->Render(context, m_shadowMaterial);
        }

        rhi.ResetRT(context, m_shadowMap);

        CBufferShadowMaps* shadowMapIndices = (CBufferShadowMaps*)m_shadowMapIndices->GetData();
        shadowMapIndices->m_shadowMapIndices[0] = m_shadowMap->GetHeapIndex();

        m_shadowMapIndices->Bind(context);
    }

    void ShadowPass::CollectRenderItems()
    {
        m_renderItems.clear();

        WeakRef<Entity> refRoot = Singleton<Scene>::Instance().GetRoot();
        CollectVisableEntity(refRoot);
    }
    void ShadowPass::CollectVisableEntity(const WeakRef<Entity>& refNode)
    {
        Ref<Entity> node = refNode.lock();
        assert(node);

        if (node->HasComponent<MeshRenderComponent>())
        {
            auto [transformer, meshRender] = node->GetComponent<TransformerComponent, MeshRenderComponent>();

            if (meshRender.m_castShadow)
            {
                UpdateCBufferPerObject(transformer, meshRender.m_renderItem->GetPerObject());
                m_renderItems.push_back(meshRender.m_renderItem);
            }
        }

        for (const WeakRef<Entity>& child : node->GetChildren())
        {
            CollectVisableEntity(child);
        }
    }
}