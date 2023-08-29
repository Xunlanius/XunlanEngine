#pragma once

#include "src/Common/Common.h"
#include "ECS/World.h"
#include "Component/Transformer.h"

#include <string>
#include <memory>
#include <set>

namespace Xunlan
{
    class Entity final
    {
        friend class Scene;

    private:

        explicit Entity(const std::string& name, const TransformerInitDesc& transformerDesc);
        DISABLE_COPY(Entity)
        DISABLE_MOVE(Entity)

    public:

        ~Entity();

    public:

        ECS::EntityID GetID() const { return m_id; }
        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }
        auto& GetChildren() { return m_children; }

        template<typename... Args>
        void AddComponent(const Args&... components) { ECS::World::Instance().AddComponent(m_id, components...); }
        template<typename... Args>
        void RemoveComponent() { ECS::World::Instance().RemoveComponent<Args...>(m_id); }
        template<typename T>
        bool HasComponent() { return ECS::World::Instance().HasComponent<T>(m_id); }
        template<typename... Args>
        std::tuple<Args&...> GetComponent() { return ECS::World::Instance().GetComponent<Args...>(m_id); }

    private:

        ECS::EntityID m_id = ECS::INVALID_ID;
        std::string m_name;

        WeakRef<Entity> m_parent;
        std::set<WeakRef<Entity>, std::owner_less<WeakRef<Entity>>> m_children;
    };
}