#if !defined SHIPPING

#include "ContentLoader.h"
#include "EngineAPI/Transformer.h"
#include "EngineAPI/Script.h"

#include <filesystem>

namespace Xunlan
{
    namespace
    {
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

            const uint32_t nameLen = *pData;
            pData += sizeof(uint32_t);

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
    }

    namespace
    {
        std::vector<EntityID> g_entities;
    }

    bool ContentLoader::LoadGame()
    {
        wchar_t path[MAX_PATH];
        const uint32_t pathLen = GetModuleFileName(0, path, MAX_PATH);

        assert(pathLen > 0);
        if (pathLen == 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER) return false;

        std::filesystem::path p(path);
        SetCurrentDirectory(p.parent_path().wstring().c_str());

        std::ifstream game("game.bin", std::ios::in, std::ios::binary);
        std::vector<byte_t> buffer(std::istreambuf_iterator<char>(game), {});

        assert(buffer.size() > 0);

        const byte_t* pData = buffer.data();

        const uint32_t numEntities = *pData;
        pData += sizeof(uint32_t);

        for (uint32_t entityIndex = 0; entityIndex < numEntities; ++entityIndex)
        {
            const uint32_t gameObjectType = *pData;
            pData += sizeof(uint32_t);

            const uint32_t numComponents = *pData;
            pData += sizeof(uint32_t);

            assert(numComponents > 0);
            if (numComponents == 0) return false;

            EntityInitDesc initDesc = {};

            for (uint32_t componentIndex = 0; componentIndex < numComponents; ++componentIndex)
            {
                const uint32_t componentType = *pData;
                pData += sizeof(uint32_t);

                assert(componentType < ComponentType::Count);
                if (!g_componentReaders[componentType](pData, initDesc)) return false;
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

        assert(pData == buffer.data() + buffer.size());

        return true;
    }
    void ContentLoader::UnloadGame() { World::GetWorld().Shutdown(); }
}

#endif