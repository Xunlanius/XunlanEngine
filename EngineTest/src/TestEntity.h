#pragma once

#include "ITest.h"

class TestGameObject : public ITest
{
public:

    virtual bool Initialize() override
    {
        srand((uint32)time(nullptr));
        return true;
    }
    virtual void Run() override
    {
        while (true)
        {
            for (size_t i = 0; i < 1000; ++i)
            {
                CreateRandom();
                RemoveRandom();
            }

            OutputResult();
        }
    }
    virtual void Shutdown() override {}

private:

    void CreateRandom()
    {
        //std::cout << "--- CreateRandom ---" << std::endl;

        //uint32 count = rand() % 100 + 100;
        uint32 count = 100;

        Xunlan::Transformer::InitInfo transformerInfo = {};
        Xunlan::Entity::InitInfo gameObjectInfo = { &transformerInfo };

        for (size_t i = 0; i < count; ++i)
        {
            ++m_numAdded;
            Xunlan::ID::id_type id = Xunlan::Entity::Create(gameObjectInfo);
            m_objects.emplace_back(id);
        }
    }
    void RemoveRandom()
    {
        //std::cout << "--- RemoveRandom ---" << std::endl;

        //uint32 count = rand() % 50 + 100;
        uint32 count = 98;

        for (size_t i = 0; i < count; ++i)
        {
            if (m_objects.size() == 0) break;

            ++m_numRemoved;
            Xunlan::ID::id_type id = m_objects.front();
            m_objects.pop_front();

            // The object should be alive before removed
            assert(Xunlan::Entity::IsAlive(id));

            Xunlan::Entity::Remove(id);

            // The object should be dead after removed
            assert(!Xunlan::Entity::IsAlive(id));
        }
    }

    void OutputResult()
    {
        std::cout << "Added: " << m_numAdded << "\n";
        std::cout << "Removed: " << m_numRemoved << "\n";
        std::cout << "Total: " << m_objects.size() << "\n" << std::endl;
    }

private:

    std::deque<Xunlan::ID::id_type> m_objects;

    size_t m_numAdded = 0;
    size_t m_numRemoved = 0;
};