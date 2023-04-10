#pragma once

#include "src/Common/Common.h"
#include <filesystem>

namespace Xunlan
{
    class ConfigSystem final
    {
        friend class Singleton<ConfigSystem>;

    private:

        ConfigSystem() = default;

    public:

        const std::filesystem::path& GetRootFolder() const { return m_rootFolder; }
        const std::filesystem::path& GetModelFolder() const { return m_modelFolder; }
        const std::filesystem::path& GetHLSLShaderFolder() const { return m_hlslShaderFolder; }
        
    private:

        std::filesystem::path m_rootFolder = "D:/Documents/Engine/Xunlan Engine/XunlanLib/";
        std::filesystem::path m_modelFolder = "D:/Documents/Engine/Xunlan Engine/XunlanLib/Asset/Model/";
        std::filesystem::path m_hlslShaderFolder = "D:/Documents/Engine/Xunlan Engine/XunlanLib/Asset/Shader/HLSL/";
    };
}