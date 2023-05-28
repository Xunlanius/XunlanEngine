#include "GeometrySystem.h"
#include "FBXImporter.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Utility/Reflection/Serializer.h"

namespace Xunlan
{
    bool GeometryImportSystem::Initialize()
    {
        m_names.push_back("Fiora");
        m_names.push_back("Plane");

        return LoadFromFBX();
    }

    void GeometryImportSystem::ImportFBX(const std::filesystem::path& filePath, MeshData& meshData)
    {
        // FBX SDK doesn't support multi-thread
        std::lock_guard lock(m_mutex);

        FBXImporter fbxImporter;
        if (!fbxImporter.Initialize(filePath.string().c_str())) return;
        if (!fbxImporter.LoadScene(meshData)) return;
    }

    bool GeometryImportSystem::LoadFromFBX()
    {
        const std::filesystem::path& modelFolder = Singleton<ConfigSystem>::Instance().GetModelFolder();

        for (const std::string& name : m_names)
        {
            const std::filesystem::path modelPath = modelFolder / (name + ".model");
            if (std::filesystem::exists(modelPath)) continue;

            const std::filesystem::path fbxPath = modelFolder / (name + ".fbx");
            if (!std::filesystem::exists(fbxPath)) { assert(false); return false; };

            MeshData meshData = {};
            ImportFBX(fbxPath, meshData);

            Reflection::BinaryStream stream;
            stream << meshData;
            stream.Save(modelPath);
        }

        return true;
    }
}