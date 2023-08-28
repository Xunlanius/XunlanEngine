#pragma once

namespace Xunlan::DX12
{
    enum class RootParam : uint32
    {
        PerObject,
        PerMaterial,
        PerFrame,

        VertexBuffer,

        MeshTextures,
        GBuffer,
        ShadowMaps,

        Count,
    };
}