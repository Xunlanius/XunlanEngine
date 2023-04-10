#pragma once

#include "src/Function/Resource/ResourceType/MeshData.h"
#include <mutex>

namespace Xunlan
{
    class GeometryImportSystem final
    {
    public:

        bool Initialize();

        void ImportFBX(const char* filePath, MeshData& meshData);

    private:

        std::mutex m_mutex;
    };
}