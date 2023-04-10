#pragma once

#include "ECS.h"
#include <iostream>

namespace Xunlan::ECS
{
    struct Component1
    {
        float position;
        float rotation;
        float scale;
    };

    struct Component2
    {
        float position;
        float rotation;
        float scale;
    };

    struct System1 : ISystem
    {
        void Update()
        {
            for (EntityID entity : m_entityIDs)
            {
                std::cout << "Update1 " << entity << std::endl;
            }
        }
    };

    struct System2 : ISystem
    {
        void Update()
        {
            for (EntityID entity : m_entityIDs)
            {
                std::cout << "Update2 " << entity << std::endl;
            }
        }
    };

    struct System3 : ISystem
    {
        void Update()
        {
            for (EntityID entity : m_entityIDs)
            {
                std::cout << "Update3 " << entity << std::endl;
            }
        }
    };

    inline void TestECS()
    {
        std::vector<EntityID> entities(10);

        RegisterComponent<Component1>();
        RegisterComponent<Component2>();

        RegisterSystem<System1, Component1>();
        RegisterSystem<System2, Component2>();
        RegisterSystem<System3, Component1, Component2>();

        for (auto& entity : entities)
        {
            entity = ECS::CreateEntity();
            ECS::AddComponent(entity, Component1());
            ECS::AddComponent(entity, Component2());
        }

        System1& system1 = GetSystem<System1>();
        System2& system2 = GetSystem<System2>();
        System3& system3 = GetSystem<System3>();

        system1.Update();
        std::cout << std::endl;
        system2.Update();
        std::cout << std::endl;
        system3.Update();
        std::cout << std::endl;

        std::cout << "___________________________________" << std::endl;
        std::cout << std::endl;

        for (auto& entity : entities)
        {
            ECS::RemoveComponent<Component2>(entity);
            //ECS::RemoveEntity(entity);
        }

        system1.Update();
        std::cout << std::endl;
        system2.Update();
        std::cout << std::endl;
        system3.Update();
    }
}