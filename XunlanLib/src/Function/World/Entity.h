#pragma once

#include "src/Common/Common.h"
#include "ECS/ECS.h"
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

    public:

        ~Entity();

    public:

        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

        auto& GetChildren() { return m_children; }

        template<typename... Args>
        void AddComponent(const Args&... components) { Singleton<ECS::ECSManager>::Instance().AddComponent(m_id, components...); }
        template<typename... Args>
        void RemoveComponent() { Singleton<ECS::ECSManager>::Instance().RemoveComponent<Args...>(m_id); }

        template<typename T>
        bool HasComponent() { return Singleton<ECS::ECSManager>::Instance().HasComponent<T>(m_id); }
        template<typename... Args>
        std::tuple<Args&...> GetComponent() { return Singleton<ECS::ECSManager>::Instance().GetComponent<Args...>(m_id); }

    private:

        ECS::EntityID m_id = ECS::INVALID_ID;
        std::string m_name;

        WeakRef<Entity> m_parent;
        std::set<WeakRef<Entity>, std::owner_less<WeakRef<Entity>>> m_children;
    };
}