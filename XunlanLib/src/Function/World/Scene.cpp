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
        return true;
    }
    void Scene::Shutdown()
    {
        m_root.reset();
        m_entities.clear();
        m_cameras.clear();
        m_lights.clear();
    }

    void Scene::LoadScene()
    {
        RHI& rhi = RHI::Instance();
        ConfigSystem& configSystem = ConfigSystem::Instance();

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
            const std::filesystem::path shaderPath = configSystem.GetHLSLFolder() / "GPass.hlsl";

            ShaderInitDesc shaderDesc = {};
            shaderDesc.m_createVS = true;
            shaderDesc.m_createPS = true;
            Ref<Shader> shader = rhi.CreateShader("PBR", shaderDesc, shaderPath);

            Ref<Material> mat0 = rhi.CreateMaterial(shader);
            Ref<Material> mat1 = rhi.CreateMaterial(shader);

            CB::PBR* cbPBR0 = mat0->GetPBRData();
            cbPBR0->m_albedo = { 1.0f, 0.0f, 0.0f, 1.0f };
            CB::PBR* cbPBR1 = mat1->GetPBRData();
            cbPBR1->m_albedo = { 1.0f, 1.0f, 1.0f, 1.0f };

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

            Ref<RenderItem> fiora = rhi.CreateRenderItem(modelFiora, { mat0 });
            Ref<RenderItem> plane0 = rhi.CreateRenderItem(modelPlane, { mat1 });
            Ref<RenderItem> plane1 = rhi.CreateRenderItem(modelPlane, { mat1 });

            const TransformerInitDesc transformerDescFiora = {
                { 0.0f, -3.0f, 0.0f },
                { -1.57f, 3.14f, 0.0f },
                { 1.0f, 1.0f, 1.0f },
            };
            const TransformerInitDesc transformerDescPlane0 = {
                { 0.0f, -5.0f, 0.0f },
                { -1.57f, 3.14f, 0.0f },
                { 0.3f, 0.3f, 0.3f },
            };
            const TransformerInitDesc transformerDescPlane1 = {
                { 2.0f, 0.0f, 0.0f },
                { 3.14f, 1.57f, 0.0f },
                { 0.3f, 0.3f, 0.3f },
            };

            Ref<Entity> entityFiora = CreateEntity("Fiora", transformerDescFiora, {}).lock();
            Ref<Entity> entityPlane0 = CreateEntity("Plane0", transformerDescPlane0, {}).lock();
            Ref<Entity> entityPlane1 = CreateEntity("Plane1", transformerDescPlane1, {}).lock();

            entityFiora->AddComponent(MeshRenderComponent{ fiora, true });
            entityPlane0->AddComponent(MeshRenderComponent{ plane0, true });
            entityPlane1->AddComponent(MeshRenderComponent{ plane1, true });
        }

        // Create light
        {
            const TransformerInitDesc transformerDesc = {
                { 0.0f, 10.0f, -10.0f },
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
}