#pragma once

#include <vector>
#include <cassert>

namespace Xunlan::Utility
{
    template<typename T>
    class ObjectPool final
    {
    public:

        ObjectPool() = default;
        explicit ObjectPool(uint32 capacity) { m_instances.reserve(capacity); }
        ~ObjectPool()
        {
            for (const uint32 id : m_cache)
            {
                T*& pData = m_instances[id];
                ::operator delete(pData);
                pData = nullptr;
            }
            for (T*& pData : m_instances)
            {
                if (!pData) continue;
                else assert(false);

                pData->~T();
                ::operator delete(pData);
                pData = nullptr;
            }
        }

    public:

        template<typename... ParamType>
        [[nodiscard]] uint32 Emplace(ParamType&&... params)
        {
            uint32 id = UINT32_MAX;
            T* pData = nullptr;

            if (!m_cache.empty())
            {
                id = m_cache.back();
                m_cache.pop_back();
                pData = m_instances[id];
            }
            else
            {
                id = m_size;
                pData = (T*)::operator new(sizeof(T));
                m_instances.push_back(pData);
            }

            new (pData) T(std::forward<ParamType>(params)...);
            ++m_size;
            return id;
        }
        void Remove(uint32& id)
        {
            assert(id < m_instances.size());
            assert(!IsRemoved(id));

            --m_size;
            m_cache.push_back(id);

            T* data = m_instances[id];
            data->~T();
            id = UINT32_MAX;
        }

        [[nodiscard]] uint32 GetSize() const { return m_size; }
        [[nodiscard]] bool IsRemoved(uint32 id)
        {
            if (id >= m_instances.size()) return true;
            return std::find(m_cache.begin(), m_cache.end(), id) != m_cache.end();
        }
        [[nodiscard]] T& Get(uint32 id)
        {
            assert(!IsRemoved(id));
            return *m_instances[id];
        }
        [[nodiscard]] const T& Get(uint32 id) const
        {
            assert(!IsRemoved(id));
            return *m_instances[id];
        }

    private:

        std::vector<T*> m_instances = {};
        std::vector<uint32> m_cache = {};
        uint32 m_size = 0;
    };

    template<typename T>
    class FreeList final
    {
        static_assert(sizeof(T) >= sizeof(uint32));

    public:

        FreeList() = default;
        explicit FreeList(uint32 capacity) { m_container.reserve(capacity); }
        ~FreeList() { assert(m_size == 0); }

    public:

        template<typename... ParamType>
        uint32 Emplace(ParamType&&... params)
        {
            uint32 id = UINT32_MAX;

            if (m_freeIndex == UINT32_MAX)
            {
                id = (uint32)m_container.size();
                m_container.emplace_back(std::forward<ParamType>(params)...);
            }
            else
            {
                id = m_freeIndex;
                m_freeIndex = *(uint32*)(std::addressof(m_container[m_freeIndex]));
                new (std::addressof(m_container[id])) T(std::forward<ParamType>(params)...);
            }

            ++m_size;
            return id;
        }
        void Remove(uint32 id)
        {
            assert(id < m_container.size());

            T& element = m_container[id];
            element.~T();
            *(uint32*)(std::addressof(element)) = m_freeIndex;
            m_freeIndex = id;
            --m_size;
        }

        uint32 GetSize() const { return m_size; }
        T& operator [](uint32 id) { return m_container[id]; }
        const T& operator [](uint32 id) const { return m_container[id]; }

    private:

        std::vector<T> m_container;
        uint32 m_size = 0;
        uint32 m_freeIndex = UINT32_MAX;
    };
}