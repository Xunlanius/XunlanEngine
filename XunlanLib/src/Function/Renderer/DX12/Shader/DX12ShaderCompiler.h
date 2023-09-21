#pragma once

#include "../DX12Common.h"
#include "d3d12shader.h"

#include <dxcapi.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace Xunlan::DX12
{
    class ShaderCompiler final
    {
    public:

        ShaderCompiler();
        DISABLE_COPY(ShaderCompiler)
        DISABLE_MOVE(ShaderCompiler)

    public:

        bool Compile(const std::filesystem::path& filePath, const std::wstring& entryPoint, const std::wstring& shaderModel);

        Microsoft::WRL::ComPtr<IDxcBlob> GetCompiledShader() const { return m_compiledShader; }
        Microsoft::WRL::ComPtr<ID3D12ShaderReflection> GetReflection() const { return m_reflection; }

    private:

        bool _Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, uint32 numArgs);

    private:

        Microsoft::WRL::ComPtr<IDxcCompiler3> m_compiler;
        Microsoft::WRL::ComPtr<IDxcUtils> m_utils;
        Microsoft::WRL::ComPtr<IDxcIncludeHandler> m_includeHandler;

        Microsoft::WRL::ComPtr<IDxcBlob> m_compiledShader;
        Microsoft::WRL::ComPtr<ID3D12ShaderReflection> m_reflection;
    };
}