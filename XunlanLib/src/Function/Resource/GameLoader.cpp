#include "GameLoader.h"
#include "src/Function/World/ECS/ECS.h"
#include "src/Function/World/Component/Transformer.h"
#include "src/Utility/IO.h"
#include "src/Utility/IOFile.h"

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
            TransformerInitDesc transformerInitDesc;
            ScriptInitDesc scriptInitDesc;
        };

        using ComponentReader = bool(*)(const byte*&, EntityInitDesc&);

        bool ReadTransformer(const byte*& pData, EntityInitDesc& initDesc)
        {
            TransformerInitDesc& transformerInitDesc = initDesc.transformerInitDesc;

            IO::ReadBuffer(&transformerInitDesc.position, pData, sizeof(transformerInitDesc.position));
            IO::ReadBuffer(&transformerInitDesc.rotation, pData, sizeof(transformerInitDesc.rotation));
            IO::ReadBuffer(&transformerInitDesc.scale, pData, sizeof(transformerInitDesc.scale));

            return true;
        }
        bool ReadScript(const byte*& pData, EntityInitDesc& initDesc)
        {
            ScriptInitDesc& scriptInitDesc = initDesc.scriptInitDesc;

            const uint32 nameLen = IO::Read<uint32>(pData);

            assert(nameLen > 0);
            assert(nameLen < 256);

            char scriptName[256] = {};
            IO::ReadBuffer(scriptName, pData, nameLen);

            scriptInitDesc.scriptCreator = GetRuntimeScriptCreator(scriptName);
            if (scriptInitDesc.scriptCreator == nullptr) return false;

            return true;
        }

        ComponentReader g_componentReaders[] = {
            ReadTransformer,
            ReadScript,
        };
        static_assert(_countof(g_componentReaders) == ComponentType::Count);
    }

    bool LoadGame()
    {
        std::unique_ptr<byte[]> buffer = nullptr;
        uint64 byteSize = 0;
        if (!IO::ReadFile("game.bin", buffer, byteSize)) return false;

        const byte* src = buffer.get();

        const uint32 numEntities = IO::Read<uint32>(src);

        for (uint32 entityIndex = 0; entityIndex < numEntities; ++entityIndex)
        {
            const uint32 gameObjectType = IO::Read<uint32>(src);
            const uint32 numComponents = IO::Read<uint32>(src);

            if (numComponents == 0) { assert(false); return false; }

            EntityInitDesc initDesc = {};

            for (uint32 comIndex = 0; comIndex < numComponents; ++comIndex)
            {
                const uint32 comType = IO::Read<uint32>(src);
                assert(comType < ComponentType::Count);
                if (!g_componentReaders[comType](src, initDesc)) return false;
            }

            TransformerComponent transformer = CreateTransformer(initDesc.transformerInitDesc);

            ECS::EntityID entity = ECS::CreateEntity(transformer);
        }

        assert(src == buffer.get() + byteSize);
        return true;
    }
    void UnloadGame() {}
}