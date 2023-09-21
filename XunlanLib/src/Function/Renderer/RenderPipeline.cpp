#include "RenderPipeline.h"
#include "Abstract/RHI.h"
#include "src/Function/World/Scene.h"
#include "src/Function/World/Component/MeshRender.h"

#include <queue>

namespace Xunlan
{
    void RenderPipeline::Initialize(uint32 width, uint32 height)
    {
        RHI& rhi = RHI::Instance();

        m_cbPerFrame = rhi.CreateCBuffer<CB::PerFrame>();
        m_cbLights = rhi.CreateCBuffer<CB::Lights>();

        m_shadowPass = std::make_unique<ShadowPass>(1024, 1024);
        m_gPass = std::make_unique<GPass>(width, height);
        m_rsmPass = std::make_unique<RSMPass>(width, height);
        m_lightingPass = std::make_unique<LightingPass>(width, height);
        m_postProcessPass = std::make_unique<PostProcessPass>();
    }
    void RenderPipeline::Shutdown()
    {
        m_cbPerFrame.reset();
        m_cbLights.reset();

        m_shadowPass.reset();
        m_gPass.reset();
        m_rsmPass.reset();
        m_lightingPass.reset();
        m_postProcessPass.reset();
    }

    void RenderPipeline::RenderForward()
    {
        UpdateItems();
        UpdateCBufferPerFrame();
        UpdateCBufferLights();

        RHI& rhi = RHI::Instance();
        Ref<RenderContext> context = rhi.CreateRenderContext();
        Scene& scene = Scene::Instance();

        context->SetParam("g_perFrame", m_cbPerFrame);
        context->SetParam("g_lights", m_cbLights);

        m_shadowPass->Render(context, m_castShadowItems);
        m_gPass->Render(context, m_items);
        m_rsmPass->Render(
            context,
            m_gPass->GetPosWS(),
            m_gPass->GetNormalWS(),
            m_shadowPass->GetFlux(),
            m_shadowPass->GetPosWS(),
            m_shadowPass->GetNormalWS()
        );
        m_lightingPass->Render(
            context,
            m_gPass->GetAlbedoWS(),
            m_gPass->GetPosWS(),
            m_gPass->GetNormalWS(),
            m_rsmPass->GetRSM(),
            m_shadowPass->GetDepthMap()
        );
        m_postProcessPass->Render(context, PostProcessEffect::NONE, m_lightingPass->GetMainRT());

        rhi.Execute(context);
        rhi.Present();

        ClearItems();
    }
    
    void RenderPipeline::UpdateItems()
    {
        WeakRef<Entity> refRoot = Scene::Instance().GetRoot();

        std::queue<WeakRef<Entity>> que;
        que.push(refRoot);

        while (!que.empty())
        {
            WeakRef<Entity> refNode = que.front();
            que.pop();

            Ref<Entity> node = refNode.lock();
            assert(node);

            if (node->HasComponent<MeshRenderComponent>())
            {
                auto [transformer, meshRender] = node->GetComponent<TransformerComponent, MeshRenderComponent>();

                UpdateCBufferPerObject(transformer, meshRender.m_renderItem->GetPerObject());

                m_items.push_back(meshRender.m_renderItem);

                if (meshRender.m_castShadow)
                {
                    m_castShadowItems.push_back(meshRender.m_renderItem);
                }
            }

            for (const WeakRef<Entity>& child : node->GetChildren())
            {
                que.push(child);
            }
        }
    }

    void RenderPipeline::ClearItems()
    {
        m_items.clear();
        m_castShadowItems.clear();
    }

    void RenderPipeline::UpdateCBufferPerFrame()
    {
        Scene& scene = Scene::Instance();

        Ref<Entity> mainCamera = scene.GetMainCamera().lock();

        assert(mainCamera && "Main camera not existed.");

        auto [transformer, camera] = mainCamera->GetComponent<TransformerComponent, CameraComponent>();
        CB::PerFrame* cbPerFrame = m_cbPerFrame->GetData<CB::PerFrame>();

        cbPerFrame->m_view = camera.m_view;
        cbPerFrame->m_proj = camera.m_projection;
        cbPerFrame->m_invProj = camera.m_invProjection;
        cbPerFrame->m_viewProj = camera.m_viewProjection;
        cbPerFrame->m_invViewProj = camera.m_invViewProjection;

        cbPerFrame->m_cameraPos = transformer.m_position;
        cbPerFrame->m_cameraDir = TransformerSystem::GetForward(transformer);
    }

    void RenderPipeline::UpdateCBufferLights()
    {
        Scene& scene = Scene::Instance();

        uint32 numDirectionalLights = 0;
        uint32 numPointLights = 0;
        uint32 numSpotLights = 0;

        CB::Lights* cbLights = m_cbLights->GetData<CB::Lights>();

        for (auto& refLight : scene.GetLights())
        {
            Ref<Entity> lightEntity = refLight.lock();
            if (!lightEntity) continue;

            auto [transformer, light] = lightEntity->GetComponent<TransformerComponent, LightComponent>();

            switch (light.m_type)
            {
            case LightType::DIRECTIONAL:
            {
                assert(numDirectionalLights == 0);

                const Math::float3 direction = TransformerSystem::GetForward(transformer);

                CB::DirectionalLight& directionalLight = cbLights->m_directionalLight;
                directionalLight.m_direction = direction;
                directionalLight.m_color = light.m_color;
                directionalLight.m_intensity = light.m_intensity;
                directionalLight.m_viewProj = GetDirectionalLightViewProj(transformer);

                ++numDirectionalLights;
            }
            break;

            case LightType::POINT:
            {
                CB::PointLight& pointLight = cbLights->m_pointLights[numPointLights];
                pointLight.m_position = transformer.m_position;
                pointLight.m_color = light.m_color;
                pointLight.m_intensity = light.m_intensity;

                ++numPointLights;
            }
            break;

            case LightType::SPOT:
            {
                CB::SpotLight& spotLight = cbLights->m_spotLights[numSpotLights];
                spotLight.m_position = transformer.m_position;
                spotLight.m_direction = TransformerSystem::GetForward(transformer);
                spotLight.m_color = light.m_color;
                spotLight.m_intensity = light.m_intensity;

                ++numSpotLights;
            }
            break;

            default: assert(false && "Unknown light type.");
            }
        }

        assert(numPointLights <= CB::MAX_NUM_POINT_LIGHTS && "Too much point lights.");

        cbLights->m_ambientLight = { 0.2f, 0.2f, 0.2f };
        cbLights->m_numPointLights = numPointLights;
        cbLights->m_numSpotLights = numSpotLights;
    }

    void RenderPipeline::UpdateCBufferPerObject(const TransformerComponent& transformer, Ref<CBuffer> cBufferPerObject)
    {
        CB::PerObject* perObject = cBufferPerObject->GetData<CB::PerObject>();
        perObject->m_world = TransformerSystem::GetWorld(transformer);
        perObject->m_invWorld = Math::GetInverse(perObject->m_world);
    }
}