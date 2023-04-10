#pragma once

#include "DataCategory.h"
#include <tuple>
#include <utility>

namespace Xunlan::Reflection
{
    template<typename MemAddress>
    struct Field final
    {
    public:

        using value_type = remove_memPtr_t<MemAddress>;

        explicit consteval Field(const char* memName, MemAddress memAddress, DataCategory category)
            : m_memName(memName), m_memAddress(memAddress), m_category(category) {}

    public:

        const char* GetMemName() const { return m_memName; }
        DataCategory GetCategory() const { return m_category; }

        template<typename Object>
        value_type& GetMember(Object& obj) const { return obj.*(m_memAddress); }
        template<typename Object>
        const value_type& GetMember(const Object& obj) const { return obj.*(m_memAddress); }

    private:

        const char* m_memName;
        const MemAddress m_memAddress;
        const DataCategory m_category;
    };

    template<typename T>
    consteval auto DefineClass();

    template<typename T>
    consteval auto DefineField(const char* memName, auto memAddress)
    {
        using MemberType = decltype(std::declval<T>().*memAddress);
        const DataCategory cateGory = DATA_CATEGORY<MemberType>;

        return Field(memName, memAddress, cateGory);
    }

#define REGISTER_CLASS(ClassName, ...) \
    template <> \
    consteval auto Xunlan::Reflection::DefineClass<ClassName>() \
    { \
        using _ClassName = ClassName; \
        return std::make_tuple(__VA_ARGS__); \
    }

#define FIELD(memName) \
    DefineField<_ClassName>(#memName, &_ClassName::##memName)

    namespace Detail
    {
        template<typename T>
        constexpr auto INSTANCE = DefineClass<std::decay_t<T>>();

        template <typename Func, typename Tuple, size_t... indices>
        constexpr void ForEachField(Tuple&& tuple, Func&& func, std::index_sequence<indices...>)
        {
            (func(std::get<indices>(std::forward<Tuple>(tuple))), ...);
        }
    }

    template<typename T, typename Func>
    constexpr void ForEachField(T&& object, Func&& func)
    {
        constexpr auto& instance = Detail::INSTANCE<std::decay_t<T>>;

        Detail::ForEachField(
            instance,
            [&object, &func](auto&& field) { func(field); },
            std::make_index_sequence<std::tuple_size_v<std::decay_t<decltype(instance)>>>()
        );
    }
}