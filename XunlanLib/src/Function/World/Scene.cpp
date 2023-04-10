#include "Scene.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Function/Renderer/RHI.h"
#include "Component/Transformer.h"
#include "Component/MeshRenderComponent.h"

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
            const CameraInitDesc cameraInitDesc(PERSPECTIVE);
            const CameraComponent camera = AddCamera(cameraInitDesc);

            const TransformerInitDesc transformerDesc = {
                { 0.0f, 0.0f, -10.0f },
                { 0.0f, 0.0f, 0.0f },
                { 1.0f, 1.0f, 1.0f },
            };

            Ref<Entity> cameraEntity = CreateEntity("Camera0", transformerDesc, {}).lock();
            cameraEntity->AddComponent(camera);
            RegisterMainCamera(cameraEntity);
        }

        // Create model
        {
            const std::filesystem::path modelPath = configSystem.GetModelFolder() / "TestModel.model";
            const std::filesystem::path pbrShaderPath = configSystem.GetHLSLShaderFolder() / "PBR.hlsl";

            ShaderList list = {};
            list.m_VS = rhi.CreateShader(ShaderType::VERTEX_SHADER, pbrShaderPath, "VS");
            list.m_PS = rhi.CreateShader(ShaderType::PIXEL_SHADER, pbrShaderPath, "PS");

            Ref<Material> pbr = rhi.CreateMaterial("PBR", MaterialType::MESH_RENDER, list);
            Ref<Mesh> model = rhi.CreateMesh(modelPath);

            model->SetNumMaterials(1);
            for (auto& submesh : model->GetSubmeshes())
            {
                submesh->SetMaterialIndex(0);
            }

            Ref<RenderItem> fiora = rhi.CreateRenderItem(model, { pbr });

            MeshRenderComponent meshRender = {};
            meshRender.m_renderItem = fiora;
            meshRender.m_castShadow = true;

            const TransformerInitDesc transformerDesc = {
                { 0.0f, -2.0f, 0.0f },
                { -1.57f, 3.14f, 0.0f },
                { 1.0f, 1.0f, 1.0f },
            };

            Ref<Entity> modelEntity = CreateEntity("Fiora", transformerDesc, {}).lock();
            modelEntity->AddComponent(meshRender);
        }
    }

    WeakRef<Entity> Scene::CreateEntity(const std::string& name, const TransformerInitDesc& transformerDesc, const WeakRef<Entity>& refParent)
    {
        std::shared_ptr<Entity> entity(new Entity(name, transformerDesc));
        Ref<Entity> parent = refParent.lock();
        if (!parent) parent = m_root;

        entity->m_parent = parent;
        parent->m_children.emplace(entity);

        m_entities.insert(entity);
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

    void Scene::UpdateCBufferPerScene()
    {
        UpdateCBufferCamera();
        UpdateCBufferLight();
    }

    void Scene::_RemoveEntity(const Ref<Entity>& entity)
    {
        for (const WeakRef<Entity>& refChild : entity->m_children)
        {
            Ref<Entity> child = refChild.lock();
            if (!child) continue;

            _RemoveEntity(child);
        }

        m_entities.erase(entity);
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
        perFrame->m_cameraDir = GetDirection(transformer);
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
                    const Math::float3 direction = GetDirection(transformer);

                    CBufferDirectionLight& directionalLight = perFrame->m_directionLights[numDirectionalLights];
                    directionalLight.m_direction = direction;
                    directionalLight.m_color = light.m_color;
                    directionalLight.m_intensity = light.m_intensity;
                    directionalLight.m_directionalLightViewProj = GetDirectionalLightViewProj(direction);

                    ++numDirectionalLights;
                    break;
                }
                case LightType::POINT:
                {
                    CBufferPointLight& pointLight = perFrame->m_pointLights[numPointLights];
                    pointLight.m_position = transformer.m_position;
                    pointLight.m_color = light.m_color;
                    pointLight.m_intensity = light.m_intensity;

                    ++numPointLights;
                    break;
                }
                case LightType::SPOT:
                {
                    CBufferSpotLight& spotLight = perFrame->m_spotLights[numSpotLights];
                    spotLight.m_position = transformer.m_position;
                    spotLight.m_direction = GetDirection(transformer);
                    spotLight.m_color = light.m_color;
                    spotLight.m_intensity = light.m_intensity;

                    ++numSpotLights;
                    break;
                }
                default: assert(false && "Unknown light type.");
            }
        }

        assert(numDirectionalLights <= MAX_NUM_DIRECTIONAL_LIGHTS && "Too much directional lights.");
        assert(numPointLights <= MAX_NUM_POINT_LIGHTS && "Too much point lights.");

        perFrame->m_ambientLight;
        perFrame->m_numDirectionalLights = numDirectionalLights;
        perFrame->m_numPointLights = numPointLights;
        perFrame->m_numSpotLights = numSpotLights;
    }
}