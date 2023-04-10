#pragma once

#include "ECSCommon.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include <typeinfo>

namespace Xunlan::ECS
{
    class ECSManager;

    class ISystem
    {
        friend class SystemManager;

    protected:

        // Entities with the needed components
        std::unordered_set<EntityID> m_entityIDs;

        ECSManager& m_manager = Singleton<ECSManager>::Instance();
    };

    class SystemManager final
    {
    public:

        template<typename T>
        void RegisterSystem(const std::unordered_set<EntityID>& entityIDs, const std::initializer_list<ComponentID>& comIDs);

        /// <returns> T* if T is registered, nullptr if T is not registered </returns>
        template<typename T>
        T& GetSystem();

        void OnEntityChanged(EntityID entityID);
        void OnEntityRemoved(EntityID entityID);

    private:

        struct SystemInfo final
        {
            ISystem* system = nullptr;
            Signature signature;
        };

        std::unordered_map<size_t, SystemInfo> m_systems;
    };

    template<typename T>
    inline void SystemManager::RegisterSystem(const std::unordered_set<EntityID>& entityIDs, const std::initializer_list<ComponentID>& comIDs)
    {
        static_assert(std::is_base_of_v<ISystem, T>, "System type should be inherited from ISystem.");

        static T system;
        const size_t hashCode = typeid(T).hash_code();

        SystemInfo info = {};
        info.system = &system;
        info.signature = Signature();

        m_systems[hashCode] = info;

        for (const ComponentID comID : comIDs)
        {
            const size_t hashCode = typeid(T).hash_code();

            Signature& signature = m_systems[hashCode].signature;
            signature.set(comID);
        }

        // Update entites to the system
        for (const EntityID entityID : entityIDs)
        {
            OnEntityChanged(entityID);
        }
    }

    template<typename T>
    inline T& SystemManager::GetSystem()
    {
        auto it = m_systems.find(typeid(T).hash_code());
        assert(it != m_systems.end() && "Non-registered system.");
        return *(T*)it->second.system;
    }

    inline void SystemManager::OnEntityChanged(EntityID entityID)
    {
        EntityManager& entityManager = Singleton<EntityManager>::Instance();

        for (const auto& [hashCode, info] : m_systems)
        {
            ISystem* system = info.system;
            Signature systemSignature = info.signature;
            assert(system);

            if (entityManager.BelongToSystem(entityID, systemSignature)) system->m_entityIDs.insert(entityID);
            else system->m_entityIDs.erase(entityID);
        }
    }

    inline void SystemManager::OnEntityRemoved(EntityID entityID)
    {
        for (const auto& [hashCode, info] : m_systems)
        {
            ISystem* system = info.system;
            assert(system);

            system->m_entityIDs.erase(entityID);
        }
    }
}