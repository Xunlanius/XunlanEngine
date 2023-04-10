#pragma once

namespace Xunlan::IO
{
    template<typename T>
    [[nodiscard]] T Read(const byte*& src)
    {
        T data = *(T*)src;
        src += sizeof(T);
        return data;
    }

    inline void ReadBuffer(void* dst, const byte*& src, size_t size)
    {
        if (size == 0) return;
        memcpy(dst, src, size);
        src += size;
    }

    template<typename T>
    void Write(const T& value, byte*& dst)
    {
        *(T*)dst = value;
        dst += sizeof(T);
    }

    inline void WriteBuffer(byte*& dst, const void* src, size_t size)
    {
        if (size == 0) return;
        memcpy(dst, src, size);
        dst += size;
    }
}