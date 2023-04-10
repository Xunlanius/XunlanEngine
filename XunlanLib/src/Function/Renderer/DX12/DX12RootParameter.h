#pragma once

namespace Xunlan::DX12
{
    enum class DefaultRootParam : uint32
    {
        PER_OBJECT,
        PER_MATERIAL,
        PER_FRAME,

        VERTEX_BUFFER,

        SHADOW_MAP_INDICES,
        TEXTURE_INDICES,

        COUNT,
    };
}