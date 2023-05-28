#pragma once

#include "ECSCommon.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include <typeinfo>

namespace Xunlan::ECS
{
    class SystemManager final
    {
    public:

        template<typename SystemType, typename... Args>
        void RegisterSystem(const std::unordered_set<EntityID>& entityIDs, Args... comIDs);

        /// <returns> T* if T is registered, nullptr if T is not registered </returns>
        template<typename T>
        std::unordered_set<EntityID>& GetView();

        void OnEntityChanged(EntityID entityID);
        void OnEntityRemoved(EntityID entityID);

    private:

        struct SystemInfo final
        {
            std::unordered_set<EntityID> m_entityIDs;
            Signature signature;
        };

        std::unordered_map<SystemID, SystemInfo> m_systems;
    };

    template<typename SystemType, typename... Args>
    inline void SystemManager::RegisterSystem(const std::unordered_set<EntityID>& entityIDs, Args... comIDs)
    {
        assert(!IDGetter<System>::IsRegistered<SystemType>());
        const SystemID systemID = IDGetter<System>::GetID<SystemType>();

        auto [it, succeed] = m_systems.emplace(systemID, SystemInfo());
        assert(succeed);
        SystemInfo& systemInfo = it->second;
        Signature& signature = systemInfo.signature;

        (signature.set(comIDs), ...);

        // Update entites to the system
        for (const EntityID entityID : entityIDs)
        {
            OnEntityChanged(entityID);
        }
    }

    template<typename T>
    inline std::unordered_set<EntityID>& SystemManager::GetView()
    {
        assert(IDGetter<System>::IsRegistered<T>() && "Non-registered system.");
        const SystemID systemID = IDGetter<System>::GetID<T>();

        auto it = m_systems.find(systemID);
        assert(it != m_systems.end());
        return it->second.m_entityIDs;
    }

    inline void SystemManager::OnEntityChanged(EntityID entityID)
    {
        EntityManager& entityManager = Singleton<EntityManager>::Instance();

        for (auto& [systemID, info] : m_systems)
        {
            std::unordered_set<EntityID>& entityIDs = info.m_entityIDs;
            Signature systemSignature = info.signature;

            if (entityManager.BelongToSystem(entityID, systemSignature)) entityIDs.insert(entityID);
            else entityIDs.erase(entityID);
        }
    }

    inline void SystemManager::OnEntityRemoved(EntityID entityID)
    {
        for (auto& [systemID, info] : m_systems)
        {
            info.m_entityIDs.erase(entityID);
        }
    }
}