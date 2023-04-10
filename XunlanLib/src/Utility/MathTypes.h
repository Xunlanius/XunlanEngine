#pragma once

namespace Xunlan::Math
{
    constexpr float EPSILON = 1e-5f;
    constexpr float PI = 3.1415926535f;
}

#if defined _WIN64
#include <DirectXMath.h>

namespace Xunlan::Math
{
    using float2 = DirectX::XMFLOAT2;
    using float3 = DirectX::XMFLOAT3;
    using float4 = DirectX::XMFLOAT4;
    using float2A = DirectX::XMFLOAT2A;
    using float3A = DirectX::XMFLOAT3A;
    using float4A = DirectX::XMFLOAT4A;

    using uint2 = DirectX::XMUINT2;
    using uint3 = DirectX::XMUINT3;
    using uint4 = DirectX::XMUINT4;

    using int2 = DirectX::XMINT2;
    using int3 = DirectX::XMINT3;
    using int4 = DirectX::XMINT4;

    using float3x3 = DirectX::XMFLOAT3X3;
    using float4x4 = DirectX::XMFLOAT4X4;
    using float4x4A = DirectX::XMFLOAT4X4A;
}
#endif