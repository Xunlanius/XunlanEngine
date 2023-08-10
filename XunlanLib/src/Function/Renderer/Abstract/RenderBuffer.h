#pragma once

#include "src/Common/Common.h"

namespace Xunlan
{
    struct RawData final
    {
        std::unique_ptr<byte[]> m_buffer;
        uint32 m_numElements;
        uint32 m_stride;
    };

    class RenderBuffer
    {
    protected:

        explicit RenderBuffer(uint32 offset, uint32 numElements, uint32 stride)
            : m_offset(offset), m_numElements(numElements), m_stride(stride) {}
        DISABLE_COPY(RenderBuffer)
        DISABLE_MOVE(RenderBuffer)
        virtual ~RenderBuffer() = default;

    public:

        uint32 GetOffset() const { return m_offset; }
        uint32 GetNumElement() const { return m_numElements; }
        uint32 GetStride() const { return m_stride; }

    private:

        uint32 m_offset;
        uint32 m_numElements;
        uint32 m_stride;
    };
}