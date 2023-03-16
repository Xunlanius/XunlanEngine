#pragma once

#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"

namespace ECS
{
    template<typename T, typename... Args>
    void RegisterSystem() { SystemManager::RegisterSystem<T, Args...>(); }
    template<typename T>
    T* GetSystem() { return SystemManager::GetSystem<T>(); }

    [[nodiscard]] inline EntityID CreateEntity() { return EntityManager::CreateEntity(); }
    inline void RemoveEntity(EntityID entity)
    {
        EntityManager::RemoveEntity(entity);
        ComponentManager::OnEntityRemoved(entity);
        SystemManager::OnEntityRemoved(entity);
    }
    
    template<typename T>
    inline void RegisterComponent() { ComponentManager::RegisterComponent<T>(); }
    template<typename T>
    inline void AddComponent(EntityID entity, const T& component)
    {
        const ComponentID comID = ComponentManager::GetComponentID<T>();
        EntityManager::GetSignature(entity).set(comID);
        ComponentManager::AddComponent<T>(entity, component);
        SystemManager::OnEntityChanged(entity);
    }
    template<typename T>
    inline void RemoveComponent(EntityID entity)
    {
        ComponentID comID = ComponentManager::GetComponentID<T>();
        EntityManager::GetSignature(entity).reset(comID);
        ComponentManager::RemoveComponent<T>(entity);
        SystemManager::OnEntityChanged(entity);
    }
    template<typename T>
    inline T& GetComponent(EntityID entity) { return ComponentManager::GetComponent<T>(entity); }
}