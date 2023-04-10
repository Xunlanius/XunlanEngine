#include "GeometrySystem.h"
#include "src/Function/Core/RuntimeContext.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "FBXImporter.h"
#include "src/Utility/Reflection/Serializer.h"

namespace Xunlan
{
    bool GeometryImportSystem::Initialize()
    {
        const std::filesystem::path& modelFolder = Singleton<ConfigSystem>::Instance().GetModelFolder();
        std::filesystem::path modelPath = modelFolder;
        modelPath += "TestModel.model";

        if (std::filesystem::exists(modelPath)) return true;

        std::filesystem::path fbxPath = modelFolder;
        fbxPath += "TestModel.fbx";

        MeshData meshData = {};
        ImportFBX(fbxPath.string().c_str(), meshData);

        Reflection::BinaryStream stream;
        stream << meshData;
        stream.Save(modelPath);

        return true;
    }

    void GeometryImportSystem::ImportFBX(const char* filePath, MeshData& meshData)
    {
        if (!filePath) { assert(false); return; }

        // FBX SDK doesn't support multi-thread
        std::lock_guard lock(m_mutex);

        FBXImporter fbxImporter;
        if (!fbxImporter.Initialize(filePath)) return;
        if (!fbxImporter.LoadScene(meshData)) return;
    }
}