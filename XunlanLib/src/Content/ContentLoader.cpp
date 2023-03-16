#if !defined SHIPPING

#include "ContentLoader.h"
#include "EngineAPI/Transformer.h"
#include "EngineAPI/Script.h"
#include "Renderer/Renderer.h"
#include "Utility/IO.h"

#include <fstream>
#include <filesystem>

namespace Xunlan
{
    namespace
    {
        bool ReadFile(std::filesystem::path path, std::unique_ptr<byte[]>& pData, uint64& byteSize)
        {
            if (!std::filesystem::exists(path)) return false;

            byteSize = std::filesystem::file_size(path);
            if (byteSize == 0) { assert(false); return false; }

            pData = std::make_unique<byte[]>(byteSize);

            std::ifstream fileStream(path, std::ios::in | std::ios::binary);
            if (!fileStream || !fileStream.read((char*)pData.get(), byteSize))
            {
                fileStream.close();
                assert(false);
                return false;
            }

            fileStream.close();
            return true;
        }

        enum ComponentType
        {
            Transformer,
            Script,
            Count,
        };

        struct EntityInitDesc
        {
            Transformer::Transformer transformer;
            Script::Script script;
        };

        bool ReadTransformer(const byte*& pData, EntityInitDesc& initInfo)
        {
            using namespace DirectX;

            Transformer::Transformer& transformer = initInfo.transformer;

            float rotation[3] = {};

            memcpy(&transformer.position, pData, sizeof(transformer.position));
            pData += sizeof(transformer.position);

            memcpy(&rotation, pData, sizeof(rotation));
            pData += sizeof(rotation);

            memcpy(&transformer.scale, pData, sizeof(transformer.scale));
            pData += sizeof(transformer.scale);

            XMFLOAT3A rot(rotation);
            XMVECTOR quat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot));

            XMFLOAT4A rotQuat;
            XMStoreFloat4A(&rotQuat, quat);
            memcpy(&transformer.rotation, &rotQuat.x, sizeof(XMFLOAT4A));

            return true;
        }
        bool ReadScript(const byte*& pData, EntityInitDesc& initInfo)
        {
            Script::Script& script = initInfo.script;

            const uint32 nameLen = Utility::Read<uint32>(pData);

            assert(nameLen > 0);
            assert(nameLen < 256);

            char scriptName[256] = {};
            memcpy(scriptName, pData, nameLen);
            pData += nameLen;

            Script::ScriptCreator creator = Script::GetRuntimeScriptCreator(scriptName);
            if (creator == nullptr)
            {
                assert(false);
                return false;
            }

            script.pScript = creator();
            return true;
        }

        using ComponentReader = bool (*)(const byte*&, EntityInitDesc&);
        ComponentReader g_componentReaders[] =
        {
            ReadTransformer,
            ReadScript,
        };
        static_assert(_countof(g_componentReaders) == ComponentType::Count);
    }

    bool ContentLoader::LoadGame()
    {
        std::unique_ptr<byte[]> pGameData = nullptr;
        uint64 byteSize = 0;
        if (!ReadFile("game.bin", pGameData, byteSize)) return false;

        const byte* at = pGameData.get();

        const uint32 numEntities = Utility::Read<uint32>(at);

        for (uint32 entityIndex = 0; entityIndex < numEntities; ++entityIndex)
        {
            const uint32 gameObjectType = Utility::Read<uint32>(at);
            const uint32 numComponents = Utility::Read<uint32>(at);

            if (numComponents == 0) { assert(false); return false; }

            EntityInitDesc initDesc = {};

            for (uint32 comIndex = 0; comIndex < numComponents; ++comIndex)
            {
                const uint32 componentType = Utility::Read<uint32>(at);
                assert(componentType < ComponentType::Count);
                if (!g_componentReaders[componentType](at, initDesc)) return false;
            }

            ECS::EntityID entity = ECS::CreateEntity();

            ECS::AddComponent<Transformer::Transformer>(entity, initDesc.transformer);
            if (initDesc.script.pScript) ECS::AddComponent<Script::Script>(entity, initDesc.script);
        }

        assert(at == pGameData.get() + byteSize);
        return true;
    }
    void ContentLoader::UnloadGame() {}

    bool ContentLoader::LoadEngineShaders(std::unique_ptr<byte[]>& shadersBlod, uint64& byteSize)
    {
        std::filesystem::path shadersBlodPath = Graphics::GetEngineShadersBlobPath();
        return ReadFile(shadersBlodPath, shadersBlod, byteSize);
    }
}

#endif