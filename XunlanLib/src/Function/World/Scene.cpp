#include "Scene.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "Component/Transformer.h"
#include "Component/NativeScript.h"
#include "Component/Light.h"
#include "Component/MeshRender.h"

#include "src/Function/Script/CameraScript.h"

namespace Xunlan
{
    bool Scene::Initialize()
    {
        m_root = Ref<Entity>(new Entity("root", {}));
        m_cBufferPerScene = RHI::Instance().CreateConstantBuffer(CBufferType::PER_FRAME, sizeof(CBufferPerFrame));
        return true;
    }
    void Scene::Shutdown()
    {
        m_root.reset();
        m_entities.clear();
        m_cameras.clear();
        m_lights.clear();

        m_cBufferPerScene.reset();
    }

    void Scene::LoadScene()
    {
        RHI& rhi = RHI::Instance();
        ConfigSystem& configSystem = Singleton<ConfigSystem>::Instance();

        // Create camera
        {
            const TransformerInitDesc transformerDesc = {
                { 0.0f, 0.0f, -10.0f },
                { 0.0f, 0.0f, 0.0f },
                { 1.0f, 1.0f, 1.0f },
            };

            Ref<Entity> cameraEntity = CreateEntity("Camera0", transformerDesc, {}).lock();
            cameraEntity->AddComponent<CameraComponent, NativeScriptComponent>({}, {});
            auto [camera, script] = cameraEntity->GetComponent<CameraComponent, NativeScriptComponent>();

            camera = CameraSystem::AddCamera(CameraInitDesc(CameraType::PERSPECTIVE));
            ScriptSystem::Bind<CameraScript>(script);

            RegisterMainCamera(cameraEntity);
        }

        // Create models
        {
            const std::filesystem::path pbrShaderPath = configSystem.GetHLSLShaderFolder() / "PBR.hlsl";

            ShaderList list = {};
            list.m_VS = rhi.CreateShader(ShaderType::VERTEX_SHADER, pbrShaderPath, "VS");
            list.m_PS = rhi.CreateShader(ShaderType::PIXEL_SHADER, pbrShaderPath, "PS");

            Ref<Material> pbr = rhi.CreateMaterial("PBR", MaterialType::MESH_RENDER, list);

            const std::filesystem::path& modelFolderPath = configSystem.GetModelFolder();
            const std::filesystem::path fioraPath = modelFolderPath / "Fiora.model";
            const std::filesystem::path planePath = modelFolderPath / "Plane.model";
            Ref<Mesh> modelFiora = rhi.CreateMesh(fioraPath);
            Ref<Mesh> modelPlane = rhi.CreateMesh(planePath);

            modelFiora->SetNumMaterials(1);
            modelPlane->SetNumMaterials(1);
            for (auto& submesh : modelFiora->GetSubmeshes())
            {
                submesh->SetMaterialIndex(0);
            }
            for (auto& submesh : modelPlane->GetSubmeshes())
            {
                submesh->SetMaterialIndex(0);
            }

            Ref<RenderItem> fiora = rhi.CreateRenderItem(modelFiora, { pbr });
            Ref<RenderItem> plane = rhi.CreateRenderItem(modelPlane, { pbr });

            const TransformerInitDesc transformerDescFiora = {
                { 0.0f, -2.0f, 0.0f },
                { -1.57f, 3.14f, 0.0f },
                { 1.0f, 1.0f, 1.0f },
            };
            const TransformerInitDesc transformerDescPlane = {
                { 0.0f, -5.0f, 0.0f },
                { -1.57f, 3.14f, 0.0f },
                { 1.0f, 1.0f, 1.0f },
            };

            Ref<Entity> entityFiora = CreateEntity("Fiora", transformerDescFiora, {}).lock();
            Ref<Entity> entityPlane = CreateEntity("Plane", transformerDescPlane, {}).lock();

            entityFiora->AddComponent(MeshRenderComponent{ fiora, true });
            entityPlane->AddComponent(MeshRenderComponent{ plane, true });
        }

        // Create light
        {
            const TransformerInitDesc transformerDesc = {
                { 0.0f, 50.0f, -50.0f },
                { 0.785f, 0.0f, 0.0f },
                { 1.0f, 1.0f, 1.0f },
            };

            Ref<Entity> entity = CreateEntity("Directional Light 0", transformerDesc, {}).lock();
            entity->AddComponent<LightComponent>({});

            auto [light] = entity->GetComponent<LightComponent>();
            light.m_type = LightType::DIRECTIONAL;
            light.m_color = { 1.0f, 1.0f, 1.0f };
            light.m_intensity = 0.7f;

            RegisterLight(entity);
        }
    }
    void Scene::OnScenePlay()
    {
        ScriptSystem::Create();
    }
    void Scene::OnSceneDestroy()
    {
        ScriptSystem::Destroy();
    }

    WeakRef<Entity> Scene::CreateEntity(const std::string& name, const TransformerInitDesc& transformerDesc, const WeakRef<Entity>& refParent)
    {
        std::shared_ptr<Entity> entity(new Entity(name, transformerDesc));
        Ref<Entity> parent = refParent.lock();
        if (!parent) parent = m_root;

        entity->m_parent = parent;
        parent->m_children.emplace(entity);

        auto [it, succeed] = m_entities.emplace(entity->GetID(), entity);
        assert(succeed);

        return entity;
    }
    void Scene::RemoveEntity(WeakRef<Entity>& refEntity)
    {
        Ref<Entity> entity = refEntity.lock();

        if (entity)
        {
            _RemoveEntity(entity);

            WeakRef<Entity>& refParent = entity->m_parent;
            Ref<Entity> parent = refParent.lock();
            assert(parent);

            parent->m_children.erase(refEntity);
        }
        
        refEntity.reset();
    }

    void Scene::RegisterMainCamera(const WeakRef<Entity>& refEntity)
    {
        Ref<Entity> cameraEntity = refEntity.lock();
        assert(cameraEntity);
        assert(cameraEntity->HasComponent<CameraComponent>());

        m_mainCamera = refEntity;
    }
    void Scene::RegisterLight(const WeakRef<Entity>& refEntity)
    {
        Ref<Entity> lightEntity = refEntity.lock();
        assert(lightEntity);
        assert(lightEntity->HasComponent<LightComponent>());

        m_lights.push_back(lightEntity);
    }

    void Scene::UpdateCBufferPerScene()
    {
        UpdateCBufferCamera();
        UpdateCBufferLight();
    }

    WeakRef<Entity> Scene::GetEntity(ECS::EntityID entityID) const
    {
        auto it = m_entities.find(entityID);
        assert(it != m_entities.end());
        return it->second;
    }

    void Scene::_RemoveEntity(const Ref<Entity>& entity)
    {
        for (const WeakRef<Entity>& refChild : entity->m_children)
        {
            const Ref<Entity> child = refChild.lock();
            if (!child) continue;

            _RemoveEntity(child);
        }

        m_entities.erase(entity->GetID());
    }

    void Scene::UpdateCBufferCamera()
    {
        Ref<Entity> mainCamera = m_mainCamera.lock();

        assert(mainCamera && "Main camera not existed.");

        auto [transformer, camera] = mainCamera->GetComponent<TransformerComponent, CameraComponent>();
        CBufferPerFrame* perFrame = (CBufferPerFrame*)m_cBufferPerScene->GetData();

        perFrame->m_view = camera.m_view;
        perFrame->m_proj = camera.m_projection;
        perFrame->m_invProj = camera.m_invProjection;
        perFrame->m_viewProj = camera.m_viewProjection;
        perFrame->m_invViewProj = camera.m_invViewProjection;

        perFrame->m_cameraPos = transformer.m_position;
        perFrame->m_cameraDir = TransformerSystem::GetForward(transformer);
    }
    void Scene::UpdateCBufferLight()
    {
        uint32 numDirectionalLights = 0;
        uint32 numPointLights = 0;
        uint32 numSpotLights = 0;

        CBufferPerFrame* perFrame = (CBufferPerFrame*)m_cBufferPerScene->GetData();

        for (auto& refLight : m_lights)
        {
            Ref<Entity> lightEntity = refLight.lock();
            if (!lightEntity) continue;

            auto [transformer, light] = lightEntity->GetComponent<TransformerComponent, LightComponent>();

            switch (light.m_type)
            {
            case LightType::DIRECTIONAL:
            {
                const Math::float3 direction = TransformerSystem::GetForward(transformer);

                CBufferDirectionLight& directionalLight = perFrame->m_directionLights[numDirectionalLights];
                directionalLight.m_direction = direction;
                directionalLight.m_color = light.m_color;
                directionalLight.m_intensity = light.m_intensity;
                directionalLight.m_viewProj = GetDirectionalLightViewProj(transformer);

                ++numDirectionalLights;
            }
            break;

            case LightType::POINT:
            {
                CBufferPointLight& pointLight = perFrame->m_pointLights[numPointLights];
                pointLight.m_position = transformer.m_position;
                pointLight.m_color = light.m_color;
                pointLight.m_intensity = light.m_intensity;

                ++numPointLights;
            }
            break;

            case LightType::SPOT:
            {
                CBufferSpotLight& spotLight = perFrame->m_spotLights[numSpotLights];
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

        assert(numDirectionalLights <= MAX_NUM_DIRECTIONAL_LIGHTS && "Too much directional lights.");
        assert(numPointLights <= MAX_NUM_POINT_LIGHTS && "Too much point lights.");

        perFrame->m_ambientLight = { 0.2f, 0.2f, 0.2f };
        perFrame->m_numDirectionalLights = numDirectionalLights;
        perFrame->m_numPointLights = numPointLights;
        perFrame->m_numSpotLights = numSpotLights;
    }
}