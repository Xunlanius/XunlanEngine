#include "ShadowPass.h"
#include "src/Function/Renderer/RHI.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Function/World/Scene.h"
#include "src/Function/World/Component/MeshRenderComponent.h"

namespace Xunlan
{
    ShadowPass::ShadowPass()
    {
        RHI& rhi = RHI::Instance();

        m_shadowMap = rhi.CreateRenderTarget(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, RenderTargetUsage::SHADOW_MAP);
        m_shadowMapIndices = rhi.CreateConstantBuffer(CBufferType::SHADOW_MAP_INDICES, sizeof(CBufferShadowMapIndices));

        ConfigSystem& configSystem = Singleton<ConfigSystem>::Instance();
        const std::filesystem::path shadowMappingShader = configSystem.GetHLSLShaderFolder() / "ShadowMapping.hlsl";

        ShaderList list = {};
        list.m_VS = rhi.CreateShader(ShaderType::VERTEX_SHADER, shadowMappingShader, "VS");

        m_shadowMaterial = rhi.CreateMaterial("Shadow_Map_Material", MaterialType::SHADOW_MAPPING, list);
    }

    void ShadowPass::Render(const Ref<RenderContext>& context)
    {
        RHI& rhi = RHI::Instance();

        rhi.SetRenderTarget(context, m_shadowMap);
        rhi.ClearRenderTarget(context, m_shadowMap);
        rhi.SetViewport(context, 0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

        CBufferShadowMapIndices* shadowMapIndices = (CBufferShadowMapIndices*)m_shadowMapIndices->GetData();
        shadowMapIndices->m_shadowMapIndices[0] = m_shadowMap->GetDepthStencilIndex();

        CollectRenderItems();

        for (const WeakRef<RenderItem>& refItem : m_renderItems)
        {
            Ref<RenderItem> item = refItem.lock();
            assert(item);
            item->Render(context, m_shadowMaterial);
        }

        rhi.ResetRenderTarget(context, m_shadowMap);

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