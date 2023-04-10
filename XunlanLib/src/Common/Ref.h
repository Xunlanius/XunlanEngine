#pragma once

#include <memory>

namespace Xunlan
{
    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T>
    using CRef = Ref<const T>;

    template<typename T>
    using WeakRef = std::weak_ptr<T>;

    template<typename T, typename... Args>
    constexpr Ref<T> MakeRef(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}