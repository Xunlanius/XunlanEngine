#pragma once

#include "ITest.h"

using namespace Xunlan;

struct Name
{
    std::string name;
};

struct HP
{
    int hp;
};

struct Timer
{
    float time;
};

void Update(Command& cmd, Queryer queryer, Resources resources)
{

}

class TestECS : public ITest
{
public:

    virtual bool Initialize() override
    {
        World& world = World::GetWorld();
        world.AddUpdateSystem(Update);

        Command cmd(world);
        EntityID entity1 = cmd.Create<Name, HP>(Name{ "Person1" }, HP{ 100 });
        EntityID entity2 = cmd.Create<Name, HP>(Name{ "Person2" }, HP{ 200 });
        EntityID entity3 = cmd.Create<Name, HP>(Name{ "Person3" }, HP{ 300 });
        ResourceID resource1 = cmd.SetResource<Timer>(Timer{ 123.0f });

        cmd.Remove(entity2);

        Queryer queryer(world);
        std::vector<EntityID> entities = queryer.Query<Name, HP>();

        std::vector<std::string> names;
        for (EntityID entity : entities)
        {
            if (queryer.HasComponent<Name>(entity))
            {
                names.push_back(queryer.GetComponent<Name>(entity).name);
            }
        }

        Resources resources(world);
        float time = 0;
        if (resources.Has<Timer>())
        {
            Timer& timer = resources.Get<Timer>();
            time = timer.time;
        }

        return true;
    }
    virtual void Run() override
    {
        World& world = World::GetWorld();
        world.OnUpdate();
    }
    virtual void Shutdown() override {}
};