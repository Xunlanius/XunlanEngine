#pragma once

namespace Xunlan
{
    class Utilities
    {
    public:

        template<typename T>
        static void EraseUnordered(std::vector<T>& vector, size_t index)
        {
            if (vector.empty()) return;
            if (vector.size() == 1) vector.clear();

            std::iter_swap(vector.begin() + index, vector.end() - 1);
            vector.pop_back();
        }
    };
}