#pragma once

#include "src/Function/Resource/ResourceType/MeshData.h"
#include <filesystem>
#include <mutex>

namespace Xunlan
{
    class GeometryImportSystem final : public Singleton<GeometryImportSystem>
    {
        friend class Singleton<GeometryImportSystem>;

    private:

        GeometryImportSystem() = default;

    public:

        bool Initialize();

        void ImportFBX(const std::filesystem::path& filePath, MeshData& meshData);

    private:

        bool LoadFromFBX();

    private:

        std::vector<std::string> m_names;

        std::mutex m_mutex;
    };
}