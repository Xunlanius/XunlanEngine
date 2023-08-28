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
    [[nodiscard]] constexpr Ref<T> MakeRef(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<class T1, class T2>
    [[nodiscard]] Ref<T1> CastTo(const Ref<T2>& rhs)
    {
        return std::dynamic_pointer_cast<T1>(rhs);
    }

    template<class T1, class T2>
    [[nodiscard]] Ref<T1> CastTo(Ref<T2>&& rhs)
    {
        return std::dynamic_pointer_cast<T1>(std::forward<Ref<T2>>(rhs));
    }
}