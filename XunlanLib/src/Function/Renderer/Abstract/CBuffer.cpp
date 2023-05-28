#include "CBuffer.h"
#include <cassert>

namespace Xunlan
{
    CBuffer::CBuffer(CBufferType type, uint32 size)
        : m_type(type), m_size(size)
    {
        assert(m_type < CBufferType::COUNT);
        m_data = std::make_unique<byte[]>(size);
    }
}