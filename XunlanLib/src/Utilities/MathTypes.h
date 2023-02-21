#pragma once

namespace Xunlan::Math
{
    constexpr float Epsilon = 1e-5f;

#if defined _WIN64

    #include <DirectXMath.h>

    using Vector2 = DirectX::XMFLOAT2;
    using Vector3 = DirectX::XMFLOAT3;
    using Vector4 = DirectX::XMFLOAT4;

    using Vector2A = DirectX::XMFLOAT2A;
    using Vector3A = DirectX::XMFLOAT3A;
    using Vector4A = DirectX::XMFLOAT4A;

    using UVector2 = DirectX::XMUINT2;
    using UVector3 = DirectX::XMUINT3;
    using UVector4 = DirectX::XMUINT4;

    using SVector2 = DirectX::XMINT2;
    using SVector3 = DirectX::XMINT3;
    using SVector4 = DirectX::XMINT4;

    using Matrix3 = DirectX::XMFLOAT3X3;
    using Matrix4 = DirectX::XMFLOAT4X4;
    using Matrix4A = DirectX::XMFLOAT4X4A;

#endif
}