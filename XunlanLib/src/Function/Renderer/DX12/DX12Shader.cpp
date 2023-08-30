#include "DX12Shader.h"
#include "d3d12shader.h"

#include <dxcapi.h>
#include <fstream>
#include <iostream>

#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;

namespace Xunlan::DX12
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

        bool Compile(ShaderType type, const std::filesystem::path& filePath, const std::string& functionName)
        {
            assert(m_compiler && m_utils && m_includeHandler);

            // Load the shader source file
            ComPtr<IDxcBlobEncoding> sourceBlob = nullptr;
            Check(m_utils->LoadFile(filePath.c_str(), nullptr, &sourceBlob));
            assert(sourceBlob && sourceBlob->GetBufferPointer() && sourceBlob->GetBufferSize() > 0);

            const std::wstring fileName = filePath.filename().wstring();
            const std::wstring funcName(functionName.begin(), functionName.end());
            const std::wstring shaderModel = ToWString(m_shaderModels[(uint32)type]);
            const std::wstring inc = filePath.parent_path().wstring();

            LPCWSTR args[] =
            {
                fileName.c_str(),
                L"-E", funcName.c_str(),    // Entry point name
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

            std::wcout << "Compile: " << fileName.c_str() << " - " << funcName.c_str() << '\n';

            return _Compile(sourceBlob.Get(), args, _countof(args));
        }

        ComPtr<IDxcBlob> GetCompiledShader() const { return m_compiledShader; }
        ComPtr<ID3D12ShaderReflection> GetReflection() const { return m_reflection; }

    private:

        bool _Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, uint32 numArgs)
        {
            DxcBuffer buffer = {};
            buffer.Ptr = sourceBlob->GetBufferPointer();
            buffer.Size = sourceBlob->GetBufferSize();
            buffer.Encoding = DXC_CP_ACP;

            // Get the compiling result
            ComPtr<IDxcResult> result;
            Check(m_compiler->Compile(&buffer, args, numArgs, m_includeHandler.Get(), IID_PPV_ARGS(&result)));

            // Get different types of the compiling result
            ComPtr<IDxcBlobUtf8> errors;
            Check(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));

            // Output compiling result
            if (errors && errors->GetStringLength() > 0)
            {
                std::wcout << "Error: " << errors->GetStringPointer() << '\n';
            }

            HRESULT status = S_OK;
            Check(result->GetStatus(&status));
            if (FAILED(status)) return false;

            ComPtr<IDxcBlob> reflectionData;
            Check(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&m_compiledShader), nullptr));
            Check(result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionData), nullptr));

            if (reflectionData)
            {
                DxcBuffer ReflectionBuffer = {};
                ReflectionBuffer.Ptr = reflectionData->GetBufferPointer();
                ReflectionBuffer.Size = reflectionData->GetBufferSize();
                ReflectionBuffer.Encoding = DXC_CP_ACP;

                m_utils->CreateReflection(&ReflectionBuffer, IID_PPV_ARGS(&m_reflection));
            }

            D3D12_SHADER_DESC shader_desc;
            m_reflection->GetDesc(&shader_desc);

            for (uint32 i = 0; i < shader_desc.BoundResources; i++)
            {
                D3D12_SHADER_INPUT_BIND_DESC  desc = {};
                m_reflection->GetResourceBindingDesc(i, &desc);

                const char* shaderVarName = desc.Name;
                uint32 bindPoint = desc.BindPoint;
                D3D_SHADER_INPUT_TYPE resourceType = desc.Type;
                uint32 registerSpace = desc.Space;

                std::cout << "    name: " << shaderVarName << std::endl;
                std::cout << "    bind point: " << bindPoint << std::endl;
                std::cout << "    resource type: " << resourceType << std::endl;
                std::cout << "    register space: " << registerSpace << std::endl;
                std::cout << std::endl;
            }

            return true;
        }

    private:

        ComPtr<IDxcCompiler3> m_compiler;
        ComPtr<IDxcUtils> m_utils;
        ComPtr<IDxcIncludeHandler> m_includeHandler;

        static constexpr const char* m_shaderModels[] = {
            "vs_6_6", "hs_6_6", "ds_6_6", "gs_6_6", "ps_6_6", "cs_6_6", "as_6_6", "ms_6_6"
        };
        static_assert(_countof(m_shaderModels) == (uint32)ShaderType::COUNT);

        ComPtr<IDxcBlob> m_compiledShader;
        ComPtr<ID3D12ShaderReflection> m_reflection;
    };

    DX12Shader::DX12Shader(ShaderType type, const void* byteCode, size_t byteCodeLength)
        : Shader(type), m_byteCodeLength(byteCodeLength)
    {
        m_byteCode = std::make_unique<byte[]>(m_byteCodeLength);
        memcpy(m_byteCode.get(), byteCode, m_byteCodeLength);
    }

    Ref<DX12Shader> DX12Shader::Create(ShaderType type, const std::filesystem::path& path, const std::string& functionName)
    {
        ShaderCompiler compiler;

        bool succeed = compiler.Compile(type, path, functionName);
        assert(succeed && "Shader compiled error.");

        ComPtr<IDxcBlob> compiledShader = compiler.GetCompiledShader();
        ComPtr<ID3D12ShaderReflection> reflection = compiler.GetReflection();

        const void* byteCode = compiledShader->GetBufferPointer();
        const size_t byteCodeLength = compiledShader->GetBufferSize();

        return MakeRef<DX12Shader>(type, byteCode, byteCodeLength);
    }
}