#pragma once

#include <vector>
#include "Common/ID.h"

namespace Xunlan
{
    template<typename T, T invalidID>
    class SparseSet
    {
    public:

        SparseSet() = default;

        void Add(T id)
        {
            assert(id != invalidID);

            m_density.push_back(id);

            for (size_t i = m_sparse.size(); i <= id; ++i)
            {
                m_sparse.push_back(invalidID);
            }

            m_sparse[id] = (T)m_density.size() - 1;
        }
        void Remove(T id)
        {
            assert(id != invalidID);
            assert(Contain(id));

            if (!Contain(id)) return;

            T& densityIndex = m_sparse[id];

            if (densityIndex != m_density.size() - 1)
            {
                m_sparse[m_density.back()] = densityIndex;
                std::swap(m_density[densityIndex], m_density.back());
                densityIndex = invalidID;
            }

            densityIndex = invalidID;
            m_density.pop_back();
        }
        bool Contain(T id)
        {
            assert(id != invalidID);
            assert(id < m_sparse.size());

            return id < m_sparse.size() && m_sparse[id] != invalidID;
        }
        void Clear()
        {
            m_density.clear();
            m_sparse.clear();
        }

        auto begin() { return m_density.begin(); }
        auto end() { return m_density.end(); }
        auto begin() const { return m_density.begin(); }
        auto end() const { return m_density.end(); }

    private:

        std::vector<T> m_density;
        std::vector<T> m_sparse;
    };
}