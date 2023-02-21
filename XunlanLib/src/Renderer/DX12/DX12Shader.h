#pragma once

#include "Renderer/DX12/DX12Common.h"

#include <dxcapi.h>
#include <d3d12shader.h>

#pragma comment(lib, "dxcompiler.lib")

namespace Xunlan::Graphics::DX12::Shader
{
    enum class ShaderType : uint32
    {
        Vertex,
        Hull,
        Domain,
        Geometry,
        Pixel,
        Compute,
        Amplification,
        Mesh,

        Count,
    };

    enum class EngineShader : uint32
    {
        TriangleVS,
        TrianglePS,
        PostProcessPS,

        Count,
    };

    bool Initialize();
    void Shutdown();

    D3D12_SHADER_BYTECODE GetEngineShaderByteCode(EngineShader id);
}