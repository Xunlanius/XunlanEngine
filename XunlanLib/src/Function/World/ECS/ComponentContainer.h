#pragma once

#include "ECSCommon.h"

namespace Xunlan::ECS
{
    class IComponentContainer
    {
    public:

        std::function<void(EntityID)> OnEntityRemovedFunc;
    };

    template<typename T>
    class ComponentContainer : public IComponentContainer
    {
    public:

        using component_type = T;

        void Emplace(EntityID entity, const component_type& component);
        void Remove(EntityID entity);

        component_type& operator[](EntityID entity) { return m_components[m_entityToIndexMap[entity]]; }

        void OnEntityRemoved(EntityID entity) { if (IsExisted(entity)) Remove(entity); }

    private:

        bool IsExisted(EntityID entity) { return m_entityToIndexMap.find(entity) != m_entityToIndexMap.end(); }

    private:

        std::deque<component_type> m_components;
        std::unordered_map<EntityID, size_t> m_entityToIndexMap;
        std::unordered_map<size_t, EntityID> m_indexToEntityMap;
    };

    template<typename T>
    inline void ComponentContainer<T>::Emplace(EntityID entity, const component_type& component)
    {
        assert(!IsExisted(entity) && "Same component added twice to an entity.");

        const size_t newIndex = m_components.size();
        m_entityToIndexMap[entity] = newIndex;
        m_indexToEntityMap[newIndex] = entity;
        m_components.push_back(component);
    }

    template<typename T>
    inline void ComponentContainer<T>::Remove(EntityID entity)
    {
        assert(IsExisted(entity) && "Removing not-existed component.");

        const size_t removedIndex = m_entityToIndexMap[entity];
        const size_t lastIndex = m_components.size() - 1;

        if (removedIndex == lastIndex)
        {
            m_components.pop_back();
            m_entityToIndexMap.erase(entity);
            m_indexToEntityMap.erase(lastIndex);
        }
        else
        {
            m_components[removedIndex] = std::move(m_components.back());
            m_components.pop_back();
            const EntityID lastEntity = m_indexToEntityMap[lastIndex];
            m_entityToIndexMap[lastEntity] = removedIndex;
            m_entityToIndexMap.erase(entity);
            m_indexToEntityMap[removedIndex] = lastEntity;
            m_indexToEntityMap.erase(lastIndex);
        }
    }
}