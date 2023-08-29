#pragma once

#include <type_traits>

namespace Xunlan
{
    template<typename T> requires std::is_class_v<T>
    class Singleton
    {
    protected:

        Singleton() = default;
        DISABLE_COPY(Singleton)
        DISABLE_MOVE(Singleton)

    public:

        static T& Instance()
        {
            static T instance;
            return instance;
        }
    };
}