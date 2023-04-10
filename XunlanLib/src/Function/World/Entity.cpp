#include "Entity.h"

namespace Xunlan
{
    Entity::Entity(const std::string& name, const TransformerInitDesc& transformerDesc)
        : m_name(name)
    {
        ECS::ECSManager& manager = Singleton<ECS::ECSManager>::Instance();
        m_id = manager.CreateEntity();
        manager.AddComponent<TransformerComponent>(m_id, CreateTransformer(transformerDesc));
    }

    Entity::~Entity()
    {
        Singleton<ECS::ECSManager>::Instance().RemoveEntity(m_id);
    }
}