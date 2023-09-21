#include "DX12ShaderCompiler.h"

#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    ShaderCompiler::ShaderCompiler()
    {
        Check(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)));
        Check(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)));
        Check(m_utils->CreateDefaultIncludeHandler(&m_includeHandler));
    }

    bool ShaderCompiler::Compile(const std::filesystem::path& filePath, const std::wstring& entryPoint, const std::wstring& shaderModel)
    {
        assert(m_compiler && m_utils && m_includeHandler);

        m_compiledShader.Reset();
        m_reflection.Reset();

        // Load the shader source file
        ComPtr<IDxcBlobEncoding> sourceBlob = nullptr;
        Check(m_utils->LoadFile(filePath.c_str(), nullptr, &sourceBlob));
        assert(sourceBlob && sourceBlob->GetBufferPointer() && sourceBlob->GetBufferSize() > 0);

        const std::wstring fileName = filePath.filename().wstring();
        const std::wstring inc = filePath.parent_path().wstring();

        LPCWSTR args[] =
        {
            fileName.c_str(),
            L"-E", entryPoint.c_str(),      // Entry point name
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

        std::wcout << "Compile: " << fileName.c_str() << " - " << entryPoint.c_str() << '\n';

        return _Compile(sourceBlob.Get(), args, _countof(args));
    }

    bool ShaderCompiler::_Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, uint32 numArgs)
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

        return true;
    }
}