#include "ShaderCompilation.h"
#include "Renderer/Renderer.h"
#include "Renderer/DX12/DX12Core.h"
#include "Renderer/DX12/DX12Shader.h"

#include <fstream>
#include <filesystem>

using namespace Xunlan;
using namespace Xunlan::Graphics::DX12;
using namespace Microsoft::WRL;

namespace
{
    struct ShaderFileInfo
    {
        const char* fileName = nullptr;
        const char* functionName = nullptr;
        Shader::EngineShader id = (Shader::EngineShader)-1;
        Shader::ShaderType type = (Shader::ShaderType)-1;
    };

    constexpr ShaderFileInfo g_shaderFileInfos[] = 
    {
        { "TriangleVS.hlsl", "TriangleVS", Shader::EngineShader::TriangleVS, Shader::ShaderType::Vertex },
        { "TrianglePS.hlsl", "TrianglePS", Shader::EngineShader::TrianglePS, Shader::ShaderType::Pixel },
        { "PostProcessPS.hlsl", "PostProcessPS", Shader::EngineShader::PostProcessPS, Shader::ShaderType::Pixel },
    };

    static_assert(_countof(g_shaderFileInfos) == (uint32)Shader::EngineShader::Count);

    constexpr const char* g_shadersSourceDir = "../../../../XunlanLib/src/Renderer/DX12/Shaders/";

    class ShaderCompiler final
    {
    public:

        ShaderCompiler()
        {
            Check(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)));
            Check(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)));
            Check(m_utils->CreateDefaultIncludeHandler(&m_includeHandler));
        }
        ShaderCompiler(const ShaderCompiler& rhs) = delete;
        ShaderCompiler& operator=(const ShaderCompiler& rhs) = delete;

    public:

        ComPtr<IDxcBlob> Compile(const ShaderFileInfo& info, const std::filesystem::path& fullPath)
        {
            assert(m_compiler && m_utils && m_includeHandler);

            // Load the shader source file
            ComPtr<IDxcBlobEncoding> sourceBlob = nullptr;
            Check(m_utils->LoadFile(fullPath.c_str(), nullptr, &sourceBlob));
            assert(sourceBlob && sourceBlob->GetBufferPointer() && sourceBlob->GetBufferSize() > 0);

            std::wstring fileName = ToWString(info.fileName);
            std::wstring functionName = ToWString(info.functionName);
            std::wstring shaderModel = ToWString(m_shaderModels[(uint32)info.type]);
            std::wstring inc = ToWString(g_shadersSourceDir);

            LPCWSTR args[] =
            {
                fileName.c_str(),
                L"-E", functionName.c_str(),    // Entry point name
                L"-T", shaderModel.c_str(),     // Set shader model
                L"-I", inc.c_str(),             // Add directory to include search path
                DXC_ARG_ALL_RESOURCES_BOUND,
#if _DEBUG
                DXC_ARG_DEBUG,
                DXC_ARG_SKIP_OPTIMIZATIONS,
#else
                DXC_ARG_OPTIMIZATION_LEVEL3,
#endif
                DXC_ARG_WARNINGS_ARE_ERRORS,
                L"-Qstrip_reflect",
                L"-Qstrip_debug",
            };

            OutputDebugStringA("Compiling ");
            OutputDebugStringA(info.fileName);
            OutputDebugStringA("...\n");

            return Compile(sourceBlob.Get(), args, _countof(args));
        }
        ComPtr<IDxcBlob> Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, uint32 numArgs)
        {
            DxcBuffer buffer = {};
            buffer.Ptr = sourceBlob->GetBufferPointer();
            buffer.Size = sourceBlob->GetBufferSize();
            buffer.Encoding = DXC_CP_ACP;

            // Get the compiling result
            ComPtr<IDxcResult> result = nullptr;
            Check(m_compiler->Compile(&buffer, args, numArgs, m_includeHandler.Get(), IID_PPV_ARGS(&result)));

            // Get different types of the compiling result
            ComPtr<IDxcBlobUtf8> errors = nullptr;
            Check(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));

            // Output compiling result
            if (errors && errors->GetStringLength() > 0)
            {
                MessageBoxA(nullptr, errors->GetStringPointer(), "Shader Compiling Error", MB_ICONERROR);
                OutputDebugStringA("Shader compiling error:\n");
                OutputDebugStringA(errors->GetStringPointer());
            }
            else
            {
                OutputDebugStringA("Succeeded");
            }
            OutputDebugStringA("\n");

            HRESULT status = S_OK;
            Check(result->GetStatus(&status));
            if (FAILED(status)) return nullptr;

            ComPtr<IDxcBlob> shader = nullptr;
            Check(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));

            return shader;
        }

    private:

        ComPtr<IDxcCompiler3> m_compiler = nullptr;
        ComPtr<IDxcUtils> m_utils = nullptr;
        ComPtr<IDxcIncludeHandler> m_includeHandler = nullptr;

        static constexpr const char* m_shaderModels[] =
        {
            "vs_6_5", "hs_6_5", "ds_6_5", "gs_6_5", "ps_6_5", "cs_6_5", "as_6_5", "ms_6_5"
        };
        static_assert(_countof(m_shaderModels) == (uint32)Shader::ShaderType::Count);
    };

    std::filesystem::path GetEngineShadersBlobPath() { return Graphics::GetEngineShadersBlobPath(Graphics::Platform::DX12); }
    // Check if we should recompile the shader source file
    bool IsCompiledShaderLatest()
    {
        std::filesystem::path engineShadersBlobPath = GetEngineShadersBlobPath();
        if (!std::filesystem::exists(engineShadersBlobPath)) { return false; }

        std::filesystem::file_time_type lastTime = std::filesystem::last_write_time(engineShadersBlobPath);

        for (uint32 i = 0; i < (uint32)Shader::EngineShader::Count; ++i)
        {
            std::filesystem::path shaderSourcePath = g_shadersSourceDir;
            shaderSourcePath += g_shaderFileInfos[i].fileName;
            shaderSourcePath = std::filesystem::absolute(shaderSourcePath);

            if (!std::filesystem::exists(shaderSourcePath)) { assert(false); return false; }

            // If any shader file was updated, it should be recompiled
            if (std::filesystem::last_write_time(shaderSourcePath) > lastTime) return false;
        }

        return true;
    }
    // Write compiled shaders to binary file
    bool WriteCompiledShaders(const std::vector<ComPtr<IDxcBlob>>& shaders)
    {
        std::filesystem::path engineShadersBlobPath = GetEngineShadersBlobPath();
        std::filesystem::path parentDir = engineShadersBlobPath.parent_path();

        if (!std::filesystem::exists(parentDir)) std::filesystem::create_directory(parentDir);

        std::ofstream stream(engineShadersBlobPath, std::ios::out | std::ios::binary);

        if (!stream || !std::filesystem::exists(engineShadersBlobPath))
        {
            stream.close();
            assert(false);
            return false;
        }

        for (const auto& shader : shaders)
        {
            const D3D12_SHADER_BYTECODE byteCode = { shader->GetBufferPointer(), shader->GetBufferSize() };
            stream.write((char*)&byteCode.BytecodeLength, sizeof(byteCode.BytecodeLength));
            stream.write((char*)byteCode.pShaderBytecode, byteCode.BytecodeLength);
        }

        stream.close();

        return true;
    }
}

bool CompileShader()
{
    if (IsCompiledShaderLatest()) return true;

    std::vector<ComPtr<IDxcBlob>> shaders;
    ShaderCompiler compiler;

    std::filesystem::path shaderSourcePath = {};

    // Put all shaders together
    for (uint32 i = 0; i < (uint32)Shader::EngineShader::Count; ++i)
    {
        const ShaderFileInfo& info = g_shaderFileInfos[i];

        shaderSourcePath = g_shadersSourceDir;
        shaderSourcePath += info.fileName;
        shaderSourcePath = absolute(shaderSourcePath);
        if (!std::filesystem::exists(shaderSourcePath)) { assert(false); return false; }

        ComPtr<IDxcBlob> compiledShader = compiler.Compile(info, shaderSourcePath);

        if (!compiledShader || !compiledShader->GetBufferPointer() || !compiledShader->GetBufferSize()) return false;

        shaders.push_back(std::move(compiledShader));
    }

    if (!WriteCompiledShaders(shaders)) return false;

    return true;
}