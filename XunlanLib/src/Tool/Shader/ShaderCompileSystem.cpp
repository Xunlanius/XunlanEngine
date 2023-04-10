#include "ShaderCompileSystem.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Function/Resource/AssetSystem.h"
#include "src/Function/Core/RuntimeContext.h"
#include "src/Utility/IO.h"
#include "src/Utility/IOFile.h"
#include "src/Utility/Reflection/Serializer.h"
#include "src/Platform/DX12/DX12Common.h"

#include <dxcapi.h>
#include <fstream>

using namespace Microsoft::WRL;

namespace Xunlan
{
    namespace DX12
    {
        class ShaderCompiler final
        {
        public:

            ShaderCompiler()
            {
                Check(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)));
                Check(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)));
                Check(m_utils->CreateDefaultIncludeHandler(&m_includeHandler));
            }
            DISABLE_COPY(ShaderCompiler)
            DISABLE_MOVE(ShaderCompiler)

        public:

            ComPtr<IDxcBlob> Compile(const std::filesystem::path& shaderFolder, const ShaderFileInfo& shaderInfo)
            {
                assert(m_compiler && m_utils && m_includeHandler);

                std::filesystem::path fullPath = shaderFolder;
                fullPath += shaderInfo.fileName;

                // Load the shader source file
                ComPtr<IDxcBlobEncoding> sourceBlob = nullptr;
                Check(m_utils->LoadFile(fullPath.c_str(), nullptr, &sourceBlob));
                assert(sourceBlob && sourceBlob->GetBufferPointer() && sourceBlob->GetBufferSize() > 0);

                std::wstring fileName = ToWString(shaderInfo.fileName);
                std::wstring functionName = ToWString(shaderInfo.functionName);
                std::wstring shaderModel = ToWString(m_shaderModels[(uint32)shaderInfo.type]);
                std::wstring inc = shaderFolder.wstring();

                LPCWSTR args[] =
                {
                    fileName.c_str(),
                    L"-E", functionName.c_str(),    // Entry point name
                    L"-T", shaderModel.c_str(),     // Set shader model
                    L"-I", inc.c_str(),             // Add directory to include search path
                    L"-enable-16bit-types",
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

                OutputDebugStringA("Compiling: ");
                OutputDebugStringA(shaderInfo.fileName);
                OutputDebugStringA(" ");
                OutputDebugStringA(shaderInfo.functionName);
                OutputDebugStringA("...\n");

                return Compile(sourceBlob.Get(), args, _countof(args));
            }

        private:

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

                ComPtr<IDxcBlob> compiledShader = nullptr;
                Check(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledShader), nullptr));

                return compiledShader;
            }

        private:

            ComPtr<IDxcCompiler3> m_compiler = nullptr;
            ComPtr<IDxcUtils> m_utils = nullptr;
            ComPtr<IDxcIncludeHandler> m_includeHandler = nullptr;

            static constexpr const char* m_shaderModels[] = {
                "vs_6_6", "hs_6_6", "ds_6_6", "gs_6_6", "ps_6_6", "cs_6_6", "as_6_6", "ms_6_6"
            };
            static_assert(_countof(m_shaderModels) == (uint32)ShaderType::Count);
        };
    }

    void ShaderCompileSystem::Initialize()
    {
        const std::filesystem::path& shaderFolder = g_runtimeContext.m_configSystem->GetHLSLShaderFolder();

        for (const EngineShaderFileInfo& info : m_registeredShaderInfos)
        {
            std::filesystem::path shaderBinPath = shaderFolder;
            shaderBinPath += info.shaderInfo.functionName;
            shaderBinPath += ".shader";

            if (!std::filesystem::exists(shaderBinPath) || !IsLastest(shaderFolder, info.shaderInfo))
            {
                CompiledShader shader = CompileShader(shaderFolder, info.shaderInfo);
                Save(shaderBinPath, shader);
            }

            LoadEngineShader(info.id, shaderBinPath);
        }
    }

    void ShaderCompileSystem::Shutdown()
    {
        for (const LongID shaderID : m_engineShaderIDs)
        {
            g_runtimeContext.m_assetSystem->UnloadShader(shaderID);
        }
    }

    CompiledShader ShaderCompileSystem::CompileShader(const std::filesystem::path& shaderFolder, const ShaderFileInfo& shaderInfo)
    {
        DX12::ShaderCompiler compiler;
        ComPtr<IDxcBlob> compiledShader = compiler.Compile(shaderFolder, shaderInfo);
        assert(compiledShader);

        uint64 byteCodeLength = compiledShader->GetBufferSize();
        const void* byteCode = compiledShader->GetBufferPointer();

        CompiledShader result = {};
        result.byteCode.resize(byteCodeLength);
        memcpy(result.byteCode.data(), byteCode, byteCodeLength);

        return result;
    }

    const CompiledShader& ShaderCompileSystem::GetEngineShader(EngineShader id)
    {
        return g_runtimeContext.m_assetSystem->GetShader(m_engineShaderIDs[(uint32)id]);
    }

    void ShaderCompileSystem::LoadEngineShader(EngineShader id, const std::filesystem::path& shaderBinPath)
    {
        Reflection::BinaryStream stream;
        stream.Load(shaderBinPath);

        CompiledShader compiledShader = {};
        stream >> compiledShader;

        LongID shaderID = g_runtimeContext.m_assetSystem->UploadShader(compiledShader);
        m_engineShaderIDs[(uint32)id] = shaderID;
    }

    bool ShaderCompileSystem::IsLastest(const std::filesystem::path& shaderFolder, const ShaderFileInfo& shaderInfo)
    {
        std::filesystem::path sourcePath = shaderFolder;
        sourcePath += shaderInfo.fileName;

        std::filesystem::path binPath = shaderFolder;
        binPath += shaderInfo.functionName;
        binPath += ".shader";

        std::filesystem::file_time_type sourceLastTime = std::filesystem::last_write_time(sourcePath);
        std::filesystem::file_time_type binLastTime = std::filesystem::last_write_time(binPath);

        return sourceLastTime < binLastTime;
    }

    void ShaderCompileSystem::Save(const std::filesystem::path& shaderBinPath, const CompiledShader& compiledShader)
    {
        /*std::ofstream stream(shaderBinPath, std::ios::out | std::ios::binary);

        const uint64 byteCodeSize = *(uint64*)compiledShader;
        stream.write((char*)compiledShader, sizeof(byteCodeSize) + byteCodeSize);

        stream.close();*/

        Reflection::BinaryStream stream;
        stream << compiledShader;
        stream.Save(shaderBinPath);
    }
}