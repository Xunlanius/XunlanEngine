#pragma once

#include "Common.h"

namespace ECS
{
    class EntityManager final
    {
    public:

        [[nodiscard]] static EntityID CreateEntity()
        {
            if (m_freeEntity == INVALID_ID)
            {
                static EntityID currID = 0;
                m_entities.insert(currID);
                m_signatures.emplace_back();
                return currID++;
            }
            
            EntityID entity = m_freeEntity;
            m_freeEntity = *(EntityID*)std::addressof(m_signatures[m_freeEntity]);
            m_entities.insert(entity);
            m_signatures[entity] = Signature();
            return entity;
        }
        static void RemoveEntity(EntityID entity)
        {
            m_entities.erase(entity);
            *(EntityID*)std::addressof(m_signatures[entity]) = m_freeEntity;
            m_freeEntity = entity;
        }

        static Signature& GetSignature(EntityID entity) { return m_signatures[entity]; }

    private:

        static inline std::unordered_set<EntityID> m_entities;
        static inline std::vector<Signature> m_signatures;
        static inline EntityID m_freeEntity = INVALID_ID;
    };
}