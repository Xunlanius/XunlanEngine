#include "Matrix.h"

#ifdef _WIN64

using namespace DirectX;

namespace Xunlan::Math
{
    float4x4 GetInverse(const float4x4& matrix)
    {
        XMMATRIX mat = XMLoadFloat4x4(&matrix);
        XMMATRIX inv = XMMatrixInverse(nullptr, mat);

        float4x4 result;
        XMStoreFloat4x4(&result, inv);

        return result;
    }
}

#endif