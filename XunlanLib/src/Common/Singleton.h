#pragma once

#include <type_traits>

namespace Xunlan
{
    template<typename T> requires std::is_class_v<T>
    class Singleton final
    {
    public:

        static T& Instance()
        {
            static T instance;
            return instance;
        }
    };
}