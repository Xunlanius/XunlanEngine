#include "Entity.h"

namespace Xunlan
{
    Entity::Entity(const std::string& name, const TransformerInitDesc& transformerDesc)
        : m_name(name)
    {
        ECS::World& manager = ECS::World::Instance();
        m_id = manager.CreateEntity();
        manager.AddComponent<TransformerComponent>(m_id, TransformerSystem::CreateTransformer(transformerDesc));
    }

    Entity::~Entity()
    {
        ECS::World::Instance().RemoveEntity(m_id);
    }
}