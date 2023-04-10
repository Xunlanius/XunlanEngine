#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/Shader.h"
#include <filesystem>

namespace Xunlan::DX12
{
    class DX12Shader final : public Shader
    {
    public:

        DX12Shader(ShaderType type, const void* byteCode, size_t byteCodeLength);

    public:

        static Ref<DX12Shader> Create(ShaderType type, const std::filesystem::path& path, const std::string& functionName);

        D3D12_SHADER_BYTECODE GetByteCode() const { return { m_byteCode.get(), m_byteCodeLength }; }

    private:

        std::unique_ptr<byte[]> m_byteCode;
        size_t m_byteCodeLength;
    };
}