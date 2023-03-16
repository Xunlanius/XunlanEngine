#pragma once

#include "Common.h"
#include "ComponentContainer.h"

namespace ECS
{
    class ComponentManager final
    {
    public:

        template<typename T>
        static void RegisterComponent();
        template<typename T>
        static ComponentID GetComponentID();
        template<typename T>
        static T& GetComponent(EntityID entity) { return GetComponentContainer<T>()[entity]; }

        template<typename T>
        static void AddComponent(EntityID entity, const T& component) { GetComponentContainer<T>().Emplace(entity, component); }
        template<typename T>
        static void RemoveComponent(EntityID entity) { GetComponentContainer<T>().Remove(entity); }
        static void OnEntityRemoved(EntityID entity) { for (auto container : ms_registeredContainer) container->OnEntityRemoved(entity); }

    private:

        template<typename T>
        static ComponentContainer<T>& GetComponentContainer();

    private:

        static inline ComponentID ms_currID = 0;
        static inline std::unordered_set<IComponentContainer*> ms_registeredContainer;
    };

    template<typename T>
    inline void ComponentManager::RegisterComponent()
    {
        static_assert(std::is_pod<T>::value, "Component type must be POD.");
        GetComponentID<T>();
        ms_registeredContainer.insert(&GetComponentContainer<T>());
    }
    template<typename T>
    inline ComponentID ComponentManager::GetComponentID()
    {
        static ComponentID comID = ms_currID++;
        return comID;
    }

    template<typename T>
    inline ComponentContainer<T>& ComponentManager::GetComponentContainer()
    {
        static ComponentContainer<T> container;
        return container;
    }
}