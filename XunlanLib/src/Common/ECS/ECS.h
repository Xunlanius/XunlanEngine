#pragma once

#include "Common/ECS/IDCore.h"
#include "Common/ECS/SparseSet.h"

namespace Xunlan
{
    class Command;
    class Queryer;
    class Resources;

    using InitializeSystem = void (*)(Command);
    using OnUpdateSystem = void (*)(Command&, Queryer, Resources);

    class World final
    {
    private:

        friend Command;
        friend Queryer;
        friend Resources;

        World() = default;

    public:

        World(const World& rhs) = delete;
        World& operator =(const World& rhs) = delete;

        static World& GetWorld()
        {
            static World instance;
            return instance;
        }

        void AddInitSystem(InitializeSystem init) { m_initFuncs.push_back(init); }
        void AddUpdateSystem(OnUpdateSystem update) { m_updateFuncs.push_back(update); }

        void Initialize();
        void OnUpdate();
        void Shutdown()
        {
            m_entityMap.clear();
            m_componentMap.clear();
            m_resourceMap.clear();
        }

    private:

        struct Pool final
        {
            std::vector<void*> instances;
            std::vector<void*> cache;

            using CreateFunc = void* (*)();
            using RemoveFunc = void (*)(void*);

            Pool(CreateFunc create, RemoveFunc remove)
                : create(create), remove(remove) {}
            ~Pool()
            {
                for (void* element : instances)
                {
                    remove(element);
                }
                for (void* element : cache)
                {
                    remove(element);
                }
            }

            template<typename T>
            T* Create(T&& element)
            {
                if (!cache.empty())
                {
                    instances.push_back(cache.back());
                    cache.pop_back();
                }
                else
                {
                    instances.push_back(create());
                }

                T* created = (T*)instances.back();
                *created = std::forward<T>(element);
                return created;
            }
            void Remove(void* element)
            {
                auto find = std::find(instances.begin(), instances.end(), element);

                assert(find != instances.end());
                if (find == instances.end()) return;

                cache.push_back(*find);
                std::swap(*find, instances.back());
                instances.pop_back();
            }

        private:

            CreateFunc create;
            RemoveFunc remove;
        };
        struct ComponentPool final
        {
            ComponentPool()
                : dataPool(nullptr, nullptr) {}
            ComponentPool(Pool::CreateFunc create, Pool::RemoveFunc remove)
                : dataPool(create, remove) {}

            // Store the EntityIDs that own this kind of component
            SparseSet<EntityID> sparseSet;
            // Store the data of this kind of component
            Pool dataPool;
        };

        using ComponentContainer = std::unordered_map<ComponentID, void*>;
        using EntityMap = std::unordered_map<EntityID, ComponentContainer>;
        using ComponentMap = std::unordered_map<ComponentID, ComponentPool>;

        struct ResourceInfo final
        {
            using CreateFunc = void* (*)();
            using RemoveFunc = void (*)(void*);

            ResourceInfo() = default;
            ResourceInfo(CreateFunc create, RemoveFunc remove)
                : create(create), remove(remove) {}
            ~ResourceInfo() { remove(pResource); }

            void* pResource = nullptr;

            CreateFunc create = nullptr;
            RemoveFunc remove = nullptr;
        };

        using ResourceMap = std::unordered_map<ResourceID, ResourceInfo>;

        EntityMap m_entityMap;
        ComponentMap m_componentMap;
        ResourceMap m_resourceMap;

        std::vector<InitializeSystem> m_initFuncs;
        std::vector<OnUpdateSystem> m_updateFuncs;
    };

    class Command final
    {
    public:

        Command(World& world)
            : m_world(world) {}

    public:

        // Create entity with certain components
        template<typename... ComponentType>
        EntityID Create(ComponentType&&... components)
        {
            EntityID entity = EntityGenerator::Generate();
            DoCreate(entity, std::forward<ComponentType>(components)...);
            return entity;
        }
        // Remove the entity and it's components
        void Remove(EntityID entity)
        {
            auto it = m_world.m_entityMap.find(entity);
            
            if (it == m_world.m_entityMap.end())
            {
                // Entity not exists
                assert(false);
            }
            else
            {
                for (auto& [componentID, component] : it->second)
                {
                    auto& componentPool = m_world.m_componentMap[componentID];
                    componentPool.sparseSet.Remove(entity);
                    componentPool.dataPool.Remove(component);
                }
                m_world.m_entityMap.erase(it);
            }
        }

        template<typename ResourceType>
        ResourceID SetResource(ResourceType&& resource)
        {
            ResourceID id = IDGetter<Resource>::Get<ResourceType>();

            auto it = m_world.m_resourceMap.find(id);
            if (it != m_world.m_resourceMap.end())
            {
                // Resource already exists
                assert(it->second.pResource);
            }
            else
            {
                auto newIt = m_world.m_resourceMap.emplace(id, World::ResourceInfo(
                    []() -> void* { return new ResourceType; },
                    [](void* element) { delete (ResourceType*)element; }));

                newIt.first->second.pResource = newIt.first->second.create();
                *(ResourceType*)newIt.first->second.pResource = std::forward<ResourceType>(resource);
            }

            return id;
        }
        template<typename T>
        void RemoveResource()
        {
            ResourceID id = IDGetter<Resource>::Get<T>();

            auto it = m_world.m_resourceMap.find(id);
            if (it == m_world.m_resourceMap.end())
            {
                // Resource not exists
                assert(false);
            }
            else
            {
                it->second.remove(it->second.pResource);
                m_world.m_resourceMap.erase(it);
            }
        }

    private:

        template<typename ComponentType, typename... Remains>
        void DoCreate(EntityID entity, ComponentType&& component, Remains&&... remains)
        {
            ComponentID componentID = IDGetter<Component>::Get<ComponentType>();

            if (m_world.m_componentMap.find(componentID) == m_world.m_componentMap.end())
            {
                m_world.m_componentMap.emplace(componentID, World::ComponentPool(
                    []() -> void* { return new ComponentType; },
                    [](void* element) { delete (ComponentType*)element; }));
            }
            
            World::ComponentPool& componentPool = m_world.m_componentMap[componentID];
            ComponentType* element = componentPool.dataPool.Create(std::forward<ComponentType>(component));
            componentPool.sparseSet.Add(entity);

            auto it = m_world.m_entityMap.emplace(entity, World::ComponentContainer());
            it.first->second[componentID] = element;

            if constexpr (sizeof...(remains) != 0)
            {
                DoCreate<Remains...>(entity, std::forward<Remains>(remains)...);
            }
        }

    private:

        World& m_world;
    };

    class Queryer final
    {
    public:

        Queryer(World& world)
            : m_world(world) {}

    public:

        template<typename ComponentType>
        bool HasComponent(EntityID entity) const
        {
            auto it = m_world.m_entityMap.find(entity);
            ComponentID id = IDGetter<Component>::Get<ComponentType>();
            return it != m_world.m_entityMap.end() && it->second.find(id) != it->second.end();
        }

        template<typename ComponentType>
        ComponentType& GetComponent(EntityID entity) const
        {
            ComponentID id = IDGetter<Component>::Get<ComponentType>();
            return *(ComponentType*)m_world.m_entityMap[entity][id];
        }

        template<typename... ComponentTypes>
        std::vector<EntityID> Query() const
        {
            std::unordered_set<EntityID> entities;
            DoQuery<ComponentTypes...>(entities);
            return std::vector<EntityID>(entities.begin(), entities.end());
        }

    private:

        template<typename ComponentType, typename... Remains>
        void DoQuery(std::unordered_set<EntityID>& out) const
        {
            ComponentID id = IDGetter<Component>::Get<ComponentType>();

            if (id >= m_world.m_componentMap.size()) return;

            World::ComponentPool& comPool = m_world.m_componentMap[id];

            for (EntityID entity : comPool.sparseSet)
            {
                if (out.find(entity) == out.end())
                    out.emplace(entity);
            }

            if constexpr (sizeof...(Remains) != 0)
            {
                DoQuery<Remains...>(out);
            }
        }

    private:

        World& m_world;
    };

    class Resources final
    {
    public:

        Resources(World& world)
            : m_world(world) {}

    public:

        template<typename ResourceType>
        bool Has() const
        {
            ResourceID id = IDGetter<Resource>::Get<ResourceType>();
            auto it = m_world.m_resourceMap.find(id);
            return it != m_world.m_resourceMap.end();
        }
        template<typename ResourceType>
        ResourceType& Get() const
        {
            ResourceID id = IDGetter<Resource>::Get<ResourceType>();
            return *(ResourceType*)m_world.m_resourceMap[id].pResource;
        }

    private:

        World& m_world;
    };

    inline void World::Initialize()
    {
        Command cmd(*this);

        for (auto init : m_initFuncs)
        {
            init(cmd);
        }
    }
    inline void World::OnUpdate()
    {
        Command cmd(*this);
        Queryer queryer(*this);
        Resources resources(*this);

        for (auto update : m_updateFuncs)
        {
            update(cmd, queryer, resources);
        }
    }
}