#pragma once

#include <iostream>

#include "ECS.h"

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

struct System1 : ECS::ISystem
{
    void Update()
    {
        for (ECS::EntityID entity : m_entities)
        {
            std::cout << "Update1 " << entity << std::endl;
        }
    }
};

struct System2 : ECS::ISystem
{
    void Update()
    {
        for (ECS::EntityID entity : m_entities)
        {
            std::cout << "Update2 " << entity << std::endl;
        }
    }
};

struct System3 : ECS::ISystem
{
    void Update()
    {
        for (ECS::EntityID entity : m_entities)
        {
            std::cout << "Update3 " << entity << std::endl;
        }
    }
};

inline void TestECS()
{
    std::vector<ECS::EntityID> entities(10);

    ECS::RegisterComponent<Component1>();
    ECS::RegisterComponent<Component2>();

    ECS::RegisterSystem<System1, Component1>();
    ECS::RegisterSystem<System2, Component2>();
    ECS::RegisterSystem<System3, Component1, Component2>();

    for (auto& entity : entities)
    {
        entity = ECS::CreateEntity();
        ECS::AddComponent(entity, Component1());
        ECS::AddComponent(entity, Component2());
    }

    System1* system1 = ECS::GetSystem<System1>();
    System2* system2 = ECS::GetSystem<System2>();
    System3* system3 = ECS::GetSystem<System3>();
    assert(system1 && system2 && system3);

    system1->Update();
    std::cout << std::endl;
    system2->Update();
    std::cout << std::endl;
    system3->Update();
    std::cout << std::endl;

    std::cout << "___________________________________" << std::endl;
    std::cout << std::endl;

    for (auto& entity : entities)
    {
        ECS::RemoveComponent<Component2>(entity);
        //ECS::RemoveEntity(entity);
    }

    system1->Update();
    std::cout << std::endl;
    system2->Update();
    std::cout << std::endl;
    system3->Update();
}