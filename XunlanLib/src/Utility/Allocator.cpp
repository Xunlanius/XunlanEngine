#include "Allocator.h"
#include <cassert>

namespace Xunlan::Utility
{
    void* BaseAllocator::Allocate(size_t size)
    {
        if (size > MAX_SIZE) return ::operator new(size);

        const size_t index = GetFreeListIndex(size);
        Obj* result = ms_freeList[index];

        if (!result) result = (Obj*)Refill(RoundUp(size));

        ms_freeList[index] = result->next;
        return result;
    }

    void BaseAllocator::Deallocate(void* ptr, size_t size)
    {
        if (size > MAX_SIZE)
        {
            ::operator delete(ptr);
            return;
        }

        Obj* p = (Obj*)ptr;

        const size_t index = GetFreeListIndex(size);
        p->next = ms_freeList[index];
        ms_freeList[index] = p;
    }

    void* BaseAllocator::Refill(size_t size)
    {
        size_t numObjs = NUM_OBJS_ALLOCATION;
        char* chunk = AllocChunk(size, numObjs);

        if (1 == numObjs)
        {
            ((Obj*)chunk)->next = nullptr;
            return chunk;
        }

        Obj*& result = ms_freeList[GetFreeListIndex(size)];
        result = (Obj*)chunk;
        Obj* curr = result;
        Obj* next = nullptr;

        for (uint32_t i = 0; i < numObjs - 1; ++i)
        {
            next = (Obj*)((char*)curr + size);
            curr->next = next;
            curr = next;
        }
        curr->next = nullptr;

        return result;
    }

    char* BaseAllocator::AllocChunk(size_t size, size_t& numObjs)
    {
        char* result = nullptr;
        const size_t requestSize = numObjs * size;
        const size_t restSize = (ms_freeStart && ms_freeEnd) ? (ms_freeEnd - ms_freeStart) : 0;

        if (requestSize <= restSize) // 剩余空间满足要求的大小
        {
            result = ms_freeStart;
            ms_freeStart += requestSize;
            return result;
        }
        else if (size <= restSize) // 剩余空间可满足一块大小
        {
            numObjs = restSize / size;
            result = ms_freeStart;
            ms_freeStart += numObjs * size;
            return result;
        }
        else // 剩余空间不足一块大小
        {
            if (restSize > 0) // 存在碎片
            {
                // 将碎片移到相应的链表上
                Obj*& fragmentTarget = ms_freeList[GetFreeListIndex(restSize)];

                ((Obj*)ms_freeStart)->next = fragmentTarget;
                fragmentTarget = (Obj*)ms_freeStart;
            }
        
            // 分配一大块内存
            size_t chunkSize = 2 * requestSize + RoundUp(ms_heapSize >> 4);
            ms_freeStart = (char*)::operator new(chunkSize);
            if (!ms_freeStart)
            {
                for (size_t i = size; i <= MAX_SIZE; i += ALIGN)
                {
                    Obj*& curr = ms_freeList[GetFreeListIndex(size)];

                    if (!curr) continue;

                    ms_freeStart = (char*)curr;
                    ms_freeEnd = ms_freeStart + i;
                    curr = curr->next;

                    return AllocChunk(size, numObjs);
                }

                assert(false); return nullptr;
            }
            ms_freeEnd = ms_freeStart + chunkSize;
            ms_heapSize += chunkSize;

            return AllocChunk(size, numObjs);
        }
    }
}