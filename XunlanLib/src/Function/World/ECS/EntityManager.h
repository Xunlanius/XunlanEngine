#pragma once

#include "ECSCommon.h"

namespace Xunlan::ECS
{
    template<typename T>
    concept ComIDConcept = std::same_as<T, ComponentID>;

    class EntityManager final : public Singleton<EntityManager>
    {
        friend class Singleton<EntityManager>;

    private:

        EntityManager() = default;

    public:

        [[nodiscard]] auto CreateEntity() -> EntityID;
        void RemoveEntity(EntityID entityID);

        const auto& GetEntities() const { return m_entities; }

        template<ComIDConcept... ComIDs>
        void AddComponent(EntityID entityID, ComIDs... comIDs) { (m_signatures[entityID].set(comIDs), ...); }
        template<ComIDConcept... ComIDs>
        void RemoveComponent(EntityID entityID, ComIDs... comIDs) { (m_signatures[entityID].reset(comIDs), ...); }

        auto HasComponent(EntityID entityID, ComponentID comID) const -> bool { return m_signatures[entityID][comID]; }
        auto BelongToSystem(EntityID entityID, Signature systemSig) -> bool { return (m_signatures[entityID] & systemSig) == systemSig; }

    private:

        std::unordered_set<EntityID> m_entities;
        std::vector<Signature> m_signatures;
        EntityID m_freeEntity = INVALID_ID;
    };

    inline EntityID EntityManager::CreateEntity()
    {
        if (m_freeEntity == INVALID_ID)
        {
            static EntityID currID = 0;
            m_entities.insert(currID);
            m_signatures.emplace_back();
            return currID++;
        }

        EntityID entityID = m_freeEntity;
        m_freeEntity = *(EntityID*)std::addressof(m_signatures[m_freeEntity]);
        m_entities.insert(entityID);
        m_signatures[entityID] = Signature();
        return entityID;
    }

    inline void EntityManager::RemoveEntity(EntityID entityID)
    {
        m_entities.erase(entityID);
        *(EntityID*)std::addressof(m_signatures[entityID]) = m_freeEntity;
        m_freeEntity = entityID;
    }
}