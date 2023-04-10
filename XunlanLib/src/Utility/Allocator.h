#pragma once

#include <cstdint>
#include <type_traits>

namespace Xunlan::Utility
{
    class BaseAllocator
    {
    public:

        [[nodiscard]] static void* Allocate(size_t size);
        static void Deallocate(void* ptr, size_t size);

        [[nodiscard]] static void* Refill(size_t size);
        [[nodiscard]] static char* AllocChunk(size_t size, size_t& numObjs);

        static size_t RoundUp(size_t size) { return (size + ALIGN - 1) & ~(ALIGN - 1); }
        static size_t GetFreeListIndex(size_t size) { return (size + ALIGN - 1) / ALIGN - 1; }

    private:

        struct Obj
        {
            Obj* next = nullptr;
        };

        static constexpr size_t ALIGN = 8;
        static constexpr size_t MAX_SIZE = 128;
        static constexpr size_t FREELIST_SIZE = MAX_SIZE / ALIGN;
        static constexpr size_t NUM_OBJS_ALLOCATION = 20;

        static inline Obj* ms_freeList[FREELIST_SIZE] = {};
        static inline char* ms_freeStart = nullptr;
        static inline char* ms_freeEnd = nullptr;
        static inline size_t ms_heapSize = 0;
    };

    template<typename T>
    class FreeList final
    {
    public:

        using id_type = uint64_t;

        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;

    public:

        template<typename... Args>
        [[nodiscard]] id_type Emplace(Args... args)
        {
            pointer ptr = (pointer)BaseAllocator::Allocate(sizeof(value_type));
            new (ptr) value_type(std::forward<Args>(args)...);
            return (id_type)ptr;
        }
        void Remove(id_type id)
        {
            pointer ptr = (pointer)id;
            if constexpr (!std::is_trivially_destructible<value_type>::value) ptr->~value_type();
            BaseAllocator::Deallocate(ptr, sizeof(value_type));
        }

        reference operator[](id_type id) { return *(pointer)id; }
        const_reference operator[](id_type id) const { return *(pointer)id; }
    };
}