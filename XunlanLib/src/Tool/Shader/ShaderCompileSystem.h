#pragma once

#include "src/Function/Resource/AssetCommon.h"
#include <filesystem>
#include <memory>

namespace Xunlan
{
    // All engine shaders should be registered here
    enum class EngineShader : uint32
    {
        TriangleVS,
        TrianglePS,
        PostProcessPS,

        Count,
    };

    struct EngineShaderFileInfo
    {
        EngineShader id = (EngineShader)-1;
        ShaderFileInfo shaderInfo;
    };

    class ShaderCompileSystem
    {
    public:

        void Initialize();
        void Shutdown();

        CompiledShader CompileShader(const std::filesystem::path& shaderFolder, const ShaderFileInfo& shaderInfo);

        const CompiledShader& GetEngineShader(EngineShader id);

    private:

        bool IsLastest(const std::filesystem::path& shaderFolder, const ShaderFileInfo& shaderInfo);
        void Save(const std::filesystem::path& shaderBinPath, const CompiledShader& compiledShader);
        void LoadEngineShader(EngineShader id, const std::filesystem::path& shaderBinPath);

    private:

        static constexpr EngineShaderFileInfo m_registeredShaderInfos[] = {
            { EngineShader::TriangleVS, { "TriangleVS.hlsl", "TriangleVS", ShaderType::Vertex } },
            { EngineShader::TrianglePS, { "TrianglePS.hlsl", "TrianglePS", ShaderType::Pixel } },
            { EngineShader::PostProcessPS, { "PostProcessPS.hlsl", "PostProcessPS", ShaderType::Pixel } },
        };
        static inline LongID m_engineShaderIDs[(uint32)EngineShader::Count];
        static_assert(_countof(m_registeredShaderInfos) == (uint32)EngineShader::Count);
    };
}