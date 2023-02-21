#pragma once

#include <vector>

namespace Xunlan
{
    template<typename T>
    class SparseSet
    {
    public:

        SparseSet() = default;

        void Add(T id)
        {
            assert(ID::IsValid(id));

            m_density.push_back(id);

            for (size_t i = m_sparse.size(); i <= id; ++i)
            {
                m_sparse.emplace_back(ID::ms_invalidID);
            }

            m_sparse[id] = (ID::id_type)m_density.size() - 1;
        }
        void Remove(T id)
        {
            assert(ID::IsValid(id));
            assert(Contain(id));

            if (!Contain(id)) return;

            T& densityIndex = m_sparse[id];

            if (densityIndex == m_density.size() - 1)
            {
                densityIndex = ID::ms_invalidID;
            }
            else
            {
                m_sparse[m_density.back()] = densityIndex;
                std::swap(m_density[densityIndex], m_density.back());
                densityIndex = ID::ms_invalidID;
            }
            m_density.pop_back();
        }
        bool Contain(T id)
        {
            assert(ID::IsValid(id));
            assert(id < m_sparse.size());

            return id < m_sparse.size() && ID::IsValid(m_sparse[id]);
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