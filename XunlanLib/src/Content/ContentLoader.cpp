#if !defined SHIPPING

#include "ContentLoader.h"
#include "EngineAPI/Transformer.h"
#include "EngineAPI/Script.h"
#include "Renderer/Renderer.h"

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

        template<typename T>
        T Read(const byte*& at)
        {
            T date = *at;
            at += sizeof(T);
            return date;
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

            const uint32 nameLen = Read<uint32>(pData);

            assert(nameLen > 0);
            assert(nameLen < 256);

            char scriptName[256] = {};
            memcpy(scriptName, pData, nameLen);
            pData += nameLen;

            Script::ScriptCreator creator = Script::IGetScriptCreator(scriptName);
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

        std::vector<EntityID> g_entities;
    }

    bool ContentLoader::LoadGame()
    {
        std::unique_ptr<byte[]> pGameData = nullptr;
        uint64 byteSize = 0;
        if (!ReadFile("game.bin", pGameData, byteSize)) return false;

        const byte* at = pGameData.get();

        const uint32 numEntities = Read<uint32>(at);

        for (uint32 entityIndex = 0; entityIndex < numEntities; ++entityIndex)
        {
            const uint32 gameObjectType = Read<uint32>(at);
            const uint32 numComponents = Read<uint32>(at);

            if (numComponents == 0) { assert(false); return false; }

            EntityInitDesc initDesc = {};

            for (uint32 comIndex = 0; comIndex < numComponents; ++comIndex)
            {
                const uint32 componentType = Read<uint32>(at);
                assert(componentType < ComponentType::Count);
                if (!g_componentReaders[componentType](at, initDesc)) return false;
            }

            Command cmd(World::GetWorld());
            EntityID entity = -1;

            if (initDesc.script.pScript)
            {
                entity = cmd.Create<Transformer::Transformer, Script::Script>(
                    std::forward<Transformer::Transformer>(initDesc.transformer),
                    std::forward<Script::Script>(initDesc.script));
            }
            else
            {
                entity = cmd.Create<Transformer::Transformer>(
                    std::forward<Transformer::Transformer>(initDesc.transformer));
            }

            g_entities.push_back(entity);
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