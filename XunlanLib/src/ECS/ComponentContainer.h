#pragma once

#include "Common.h"

namespace ECS
{
    class IComponentContainer
    {
    public:

        virtual void OnEntityRemoved(EntityID entity) = 0;
    };

    template<typename T>
    class ComponentContainer : public IComponentContainer
    {
    public:

        using component_type = T;

        void Emplace(EntityID entity, const component_type& component);
        void Remove(EntityID entity);

        virtual void OnEntityRemoved(EntityID entity) override { if (IsExisted(entity)) Remove(entity); }

        component_type& operator[](EntityID entity) { return m_components[m_entityToIndexMap[entity]]; }
        component_type& operator[](EntityID entity) const { return m_components[m_entityToIndexMap[entity]]; }

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
        assert(!IsExisted(entity) && "Same type of component added twice to entity.");

        size_t newIndex = m_components.size();
        m_entityToIndexMap[entity] = newIndex;
        m_indexToEntityMap[newIndex] = entity;
        m_components.push_back(component);
    }

    template<typename T>
    inline void ComponentContainer<T>::Remove(EntityID entity)
    {
        assert(IsExisted(entity) && "Removing non-existed component.");

        size_t removedIndex = m_entityToIndexMap[entity];
        size_t lastIndex = m_components.size() - 1;
        m_components[removedIndex] = m_components.back();
        m_components.pop_back();
        EntityID lastEntity = m_indexToEntityMap[lastIndex];
        m_entityToIndexMap[lastEntity] = removedIndex;
        m_entityToIndexMap.erase(entity);
        m_indexToEntityMap[removedIndex] = lastEntity;
        m_indexToEntityMap.erase(lastIndex);
    }
}