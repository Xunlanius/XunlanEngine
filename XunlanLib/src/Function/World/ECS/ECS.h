#pragma once

#include "src/Common/Singleton.h"
#include "src/Common/Macro.h"
#include "ECSCommon.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"

namespace Xunlan::ECS
{
    class ECSManager final
    {
    private:

        friend class Singleton<ECSManager>;
        ECSManager() = default;
        DISABLE_COPY(ECSManager)
        DISABLE_MOVE(ECSManager)

    public:

        template<typename T, typename... Args>
        void RegisterSystem();

        [[nodiscard]] EntityID CreateEntity() { return m_entityManager.CreateEntity(); }
        void RemoveEntity(EntityID& entityID);
        void ExecuteRemoveEntities() {}

        template<typename T>
        bool HasComponent(EntityID entityID);
        template<typename... Args>
        std::tuple<Args&...> GetComponent(EntityID entityID);

        template<typename... Args>
        void AddComponent(EntityID entityID, const Args&... components);
        template<typename... Args>
        void RemoveComponent(EntityID entityID);

        template<typename T>
        T& GetSystem() { return m_systemManager.GetSystem<T>(); }

    private:

        EntityManager& m_entityManager = Singleton<EntityManager>::Instance();
        ComponentManager m_componentManager;
        SystemManager m_systemManager;
    };

    template<typename T, typename... Args>
    void ECSManager::RegisterSystem()
    {
        static_assert(sizeof...(Args) > 0, "System needs at least one component.");

        (m_componentManager.RegisterComponent<Args>(), ...);
        m_systemManager.RegisterSystem<T>(m_entityManager.GetEntities(), { m_componentManager.GetComponentID<Args>()... });
    }

    inline void ECSManager::RemoveEntity(EntityID& entityID)
    {
        m_entityManager.RemoveEntity(entityID);
        m_componentManager.OnEntityRemoved(entityID);
        m_systemManager.OnEntityRemoved(entityID);
        entityID = INVALID_ID;
    }

    template<typename T>
    bool ECSManager::HasComponent(EntityID entityID)
    {
        if (!m_componentManager.IsRegistered<T>()) return false;

        const ComponentID comID = m_componentManager.GetComponentID<T>();
        return m_entityManager.HasComponent(entityID, comID);
    }

    template<typename... Args>
    std::tuple<Args&...> ECSManager::GetComponent(EntityID entityID)
    {
        assert(HasComponent<Args>(entityID) && ...);
        return m_componentManager.GetComponent<Args...>(entityID);
    }

    template<typename... Args>
    void ECSManager::AddComponent(EntityID entityID, const Args&... components)
    {
        assert((!HasComponent<Args>(entityID)) && ...);

        m_entityManager.AddComponent(entityID, m_componentManager.GetComponentID<Args>()...);
        m_componentManager.AddComponent(entityID, components...);
        m_systemManager.OnEntityChanged(entityID);
    }

    template<typename... Args>
    void ECSManager::RemoveComponent(EntityID entityID)
    {
        assert(HasComponent<Args>(entityID) && ...);

        m_entityManager.RemoveComponent(entityID, m_componentManager.GetComponentID<Args>()...);
        m_componentManager.RemoveComponent<Args...>(entityID);
        m_systemManager.OnEntityChanged(entityID);
    }
}