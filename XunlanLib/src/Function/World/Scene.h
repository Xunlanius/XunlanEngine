#pragma once

#include "src/Common/Common.h"
#include "Entity.h"
#include "Component/Camera.h"
#include "Component/Light.h"
#include "src/Function/Renderer/Abstract/CBuffer.h"

#include <vector>
#include <unordered_map>

namespace Xunlan
{
    class Scene final : public Singleton<Scene>
    {
        friend class Singleton<Scene>;

    private:

        Scene() = default;

    public:

        bool Initialize();
        void Shutdown();

        void LoadScene();
        void OnScenePlay();
        void OnSceneDestroy();

        WeakRef<Entity> CreateEntity(const std::string& name, const TransformerInitDesc& transformerDesc, const WeakRef<Entity>& refParent);
        void RemoveEntity(WeakRef<Entity>& refEntity);

        void RegisterMainCamera(const WeakRef<Entity>& refEntity);
        void RegisterLight(const WeakRef<Entity>& refEntity);

        void UpdateCBufferPerScene();

        WeakRef<Entity> GetEntity(ECS::EntityID entityID) const;
        WeakRef<Entity> GetRoot() const { return m_root; }
        Ref<CBuffer> GetCBufferPerScene() const { return m_cBufferPerScene; }

    private:

        void _RemoveEntity(const Ref<Entity>& entity);

        void UpdateCBufferCamera();
        void UpdateCBufferLight();

    private:

        Ref<Entity> m_root;
        std::unordered_map<ECS::EntityID, Ref<Entity>> m_entities;

        WeakRef<Entity> m_mainCamera;
        std::vector<WeakRef<Entity>> m_cameras;
        std::vector<WeakRef<Entity>> m_lights;

        Ref<CBuffer> m_cBufferPerScene;
    };
}