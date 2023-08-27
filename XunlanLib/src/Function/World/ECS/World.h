#pragma once

#include "src/Common/Singleton.h"
#include "src/Common/Macro.h"
#include "ECSCommon.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"

namespace Xunlan::ECS
{
    class World final
    {
        friend class Singleton<World>;

    private:

        World() = default;
        DISABLE_COPY(World)
        DISABLE_MOVE(World)
        ~World() = default;

    public:

        template<typename SystemType, typename... ComArgs>
        void RegisterSystem();

        [[nodiscard]] EntityID CreateEntity() { return m_entityManager.CreateEntity(); }
        void RemoveEntity(EntityID& entityID);
        void ExecuteRemoveEntities() {}

        template<ComponentConcept T>
        bool HasComponent(EntityID entityID);
        template<ComponentConcept... Args>
        std::tuple<Args&...> GetComponent(EntityID entityID);

        template<ComponentConcept... Args>
        void AddComponent(EntityID entityID, const Args&... components);
        template<ComponentConcept... Args>
        void RemoveComponent(EntityID entityID);

        template<typename SystemType>
        std::unordered_set<EntityID>& GetView() { return m_systemManager.GetView<SystemType>(); }

    private:

        EntityManager& m_entityManager = Singleton<EntityManager>::Instance();
        ComponentManager m_componentManager;
        SystemManager m_systemManager;
    };

    template<typename SystemType, typename... ComArgs>
    void World::RegisterSystem()
    {
        static_assert(sizeof...(ComArgs) > 0, "System needs at least one component.");

        (m_componentManager.RegisterComponent<ComArgs>(), ...);
        m_systemManager.RegisterSystem<SystemType>(m_entityManager.GetEntities(), IDGetter<Component>::GetID<ComArgs>()...);
    }

    inline void World::RemoveEntity(EntityID& entityID)
    {
        m_entityManager.RemoveEntity(entityID);
        m_componentManager.OnEntityRemoved(entityID);
        m_systemManager.OnEntityRemoved(entityID);
        entityID = INVALID_ID;
    }

    template<ComponentConcept T>
    bool World::HasComponent(EntityID entityID)
    {
        if (!IDGetter<Component>::IsRegistered<T>()) return false;

        const ComponentID comID = IDGetter<Component>::GetID<T>();
        return m_entityManager.HasComponent(entityID, comID);
    }

    template<ComponentConcept... Args>
    std::tuple<Args&...> World::GetComponent(EntityID entityID)
    {
        assert(HasComponent<Args>(entityID) && ...);
        return m_componentManager.GetComponent<Args...>(entityID);
    }

    template<ComponentConcept... Args>
    void World::AddComponent(EntityID entityID, const Args&... components)
    {
        assert(!HasComponent<Args>(entityID) && ...);

        m_entityManager.AddComponent(entityID, IDGetter<Component>::GetID<Args>()...);
        m_componentManager.AddComponent(entityID, components...);
        m_systemManager.OnEntityChanged(entityID);
    }

    template<ComponentConcept... Args>
    void World::RemoveComponent(EntityID entityID)
    {
        assert(HasComponent<Args>(entityID) && ...);

        m_entityManager.RemoveComponent(entityID, IDGetter<Component>::GetID<Args>()...);
        m_componentManager.RemoveComponent<Args...>(entityID);
        m_systemManager.OnEntityChanged(entityID);
    }
}