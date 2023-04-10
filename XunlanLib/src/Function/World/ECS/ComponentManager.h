#pragma once

#include "ECSCommon.h"
#include "ComponentContainer.h"

namespace Xunlan::ECS
{
    template<typename T>
    concept ComponentConcept = requires
    {
        requires std::is_class_v<T>;
        requires !std::is_reference_v<T>;
        requires !std::is_pointer_v<T>;
    };

    class ComponentManager final
    {
    public:

        template<ComponentConcept T>
        void RegisterComponent();
        template<ComponentConcept T>
        ComponentID GetComponentID();
        template<ComponentConcept T>
        bool IsRegistered();

        template<ComponentConcept... Args>
        std::tuple<Args&...> GetComponent(EntityID entity);
        template<ComponentConcept T, ComponentConcept... Args>
        void AddComponent(EntityID entity, const T& component, const Args&... args);
        template<ComponentConcept T, ComponentConcept... Args>
        void RemoveComponent(EntityID entity);

        void OnEntityRemoved(EntityID entity);

    private:

        template<ComponentConcept T>
        ComponentContainer<T>& GetComponentContainer();

    private:

        ComponentID m_currID = 0;
        std::unordered_set<IComponentContainer*> m_containers;
    };

    template<ComponentConcept T>
    inline void ComponentManager::RegisterComponent()
    {
        GetComponentID<T>();
        ComponentContainer<T>& container = GetComponentContainer<T>();
        container.OnEntityRemovedFunc = [&container](EntityID entity) { container.OnEntityRemoved(entity); };
        m_containers.insert(&container);
    }
    template<ComponentConcept T>
    inline ComponentID ComponentManager::GetComponentID()
    {
        static const ComponentID comID = m_currID++;
        return comID;
    }
    template<ComponentConcept T>
    inline bool ComponentManager::IsRegistered()
    {
        const ComponentID currComID = m_currID;
        return GetComponentID<T>() < currComID;
    }

    template<ComponentConcept... Args>
    inline std::tuple<Args&...> ComponentManager::GetComponent(EntityID entity)
    {
        return std::tuple<Args&...>(GetComponentContainer<Args>()[entity]...);
    }

    template<ComponentConcept T, ComponentConcept... Args>
    inline void ComponentManager::AddComponent(EntityID entity, const T& component, const Args&... args)
    {
        if (IsRegistered<T>()) RegisterComponent<T>();
        GetComponentContainer<T>().Emplace(entity, component);

        if constexpr (sizeof...(Args) > 0) AddComponent(entity, args...);
    }

    template<ComponentConcept T, ComponentConcept... Args>
    inline void ComponentManager::RemoveComponent(EntityID entity)
    {
        GetComponentContainer<T>().Remove(entity);

        if constexpr (sizeof...(Args) > 0) RemoveComponent<Args...>(entity);
    }

    inline void ComponentManager::OnEntityRemoved(EntityID entity)
    {
        for (auto container : m_containers)
        {
            container->OnEntityRemovedFunc(entity);
        }
    }

    template<ComponentConcept T>
    inline ComponentContainer<T>& ComponentManager::GetComponentContainer()
    {
        static ComponentContainer<T> container;
        return container;
    }
}