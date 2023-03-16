#pragma once

#include "Common.h"
#include "EntityManager.h"
#include "ComponentManager.h"

namespace ECS
{
    struct ISystem
    {
        // Entities with the needed components
        std::unordered_set<EntityID> m_entities;
    };

    class SystemManager final
    {
    public:

        template<typename T, typename... Args>
        static void RegisterSystem();

        /// <returns> T* if T is registered, nullptr if T is not registered </returns>
        template<typename T>
        static T* GetSystem();

        static void OnEntityChanged(EntityID entity);
        static void OnEntityRemoved(EntityID entity);

    private:

        template<typename T, typename Arg, typename... Args>
        static void RegisterSystemInternal();

        static inline std::unordered_map<size_t, ISystem*> ms_registeredSystems;
        static inline std::unordered_map<size_t, Signature> ms_registeredSignatures;
    };

    template<typename T, typename... Args>
    inline void SystemManager::RegisterSystem()
    {
        static_assert(std::is_convertible<T, ISystem>::value, "System type should be inherited from ISystem.");
        static_assert(sizeof...(Args) > 0, "System needs at least one component.");

        static T system;
        const size_t hashCode = typeid(T).hash_code();

        ms_registeredSystems[hashCode] = &system;
        ms_registeredSignatures[hashCode] = Signature();

        RegisterSystemInternal<T, Args...>();
    }

    template<typename T, typename Arg, typename... Args>
    inline void SystemManager::RegisterSystemInternal()
    {
        ComponentManager::RegisterComponent<Arg>();
        const ComponentID comID = ComponentManager::GetComponentID<Arg>();
        const size_t hashCode = typeid(T).hash_code();
        ms_registeredSignatures[hashCode].set(comID);

        if constexpr (sizeof...(Args) != 0) RegisterSystemInternal<T, Args...>();
    }

    template<typename T>
    inline T* SystemManager::GetSystem()
    {
        auto it = ms_registeredSystems.find(typeid(T).hash_code());
        return it != ms_registeredSystems.end() ? (T*)it->second : nullptr;
    }

    inline void SystemManager::OnEntityChanged(EntityID entity)
    {
        for (const auto& [hashCode, signature] : ms_registeredSignatures)
        {
            ISystem* system = ms_registeredSystems[hashCode];
            assert(system);

            if ((EntityManager::GetSignature(entity) & signature) == signature) system->m_entities.insert(entity);
            else system->m_entities.erase(entity);
        }
    }
    inline void SystemManager::OnEntityRemoved(EntityID entity)
    {
        for (const auto& [hashCode, signature] : ms_registeredSignatures)
        {
            ISystem* system = ms_registeredSystems[hashCode];
            assert(system);

            system->m_entities.erase(entity);
        }
    }
}