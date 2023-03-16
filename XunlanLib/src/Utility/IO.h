#pragma once

namespace Xunlan::Utility
{
    template<typename T>
    [[nodiscard]] T Read(const byte*& at)
    {
        T data = *(T*)at;
        at += sizeof(T);
        return data;
    }

    inline void Read(void* dst, const byte*& src, size_t size)
    {
        memcpy(dst, src, size);
        src += size;
    }

    template<typename T>
    void Write(const T& value, byte*& at)
    {
        *(T*)at = value;
        at += sizeof(T);
    }

    inline void Write(byte*& dst, const void* src, size_t size)
    {
        memcpy(dst, src, size);
        dst += size;
    }
}