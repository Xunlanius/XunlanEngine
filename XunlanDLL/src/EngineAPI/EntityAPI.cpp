#include "Common.h"
#include "EngineAPI/Transformer.h"
#include "EngineAPI/Script.h"

using namespace Xunlan;

namespace
{
    // Given by the "Editor"

    struct TransformerEditor
    {
        float position[3] = {};
        float rotation[3] = {};
        float scale[3] = { 1.0f, 1.0f, 1.0f };

        Transformer::Transformer ToEngineInfo() const
        {
            using namespace DirectX;

            Transformer::Transformer transformer = {};

            transformer.position = Math::Vector3(position);
            transformer.scale = Math::Vector3(scale);

            XMFLOAT3A rot(rotation);
            XMVECTOR quat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot));

            XMFLOAT4A rotQuat;
            XMStoreFloat4A(&rotQuat, quat);
            transformer.rotation = rotQuat;

            return transformer;
        }
    };

    struct ScriptEditor
    {
        Script::ScriptCreator scriptCreator;

        Script::Script ToEngineInfo() const
        {
            Script::Script script = {};
            script.pScript = scriptCreator ? scriptCreator() : nullptr;
            return script;
        }
    };

    struct EntityInitInfo
    {
        TransformerEditor transformer;
        ScriptEditor script;
    };
}

EDITOR_INTERFACE ECS::EntityID Create(const EntityInitInfo& desc)
{
    Transformer::Transformer transformer = desc.transformer.ToEngineInfo();
    Script::Script script = desc.script.ToEngineInfo();

    ECS::EntityID entity = ECS::CreateEntity();

    ECS::AddComponent<Transformer::Transformer>(entity, transformer);
    if (script.pScript) ECS::AddComponent<Script::Script>(entity, script);

    return entity;
}
EDITOR_INTERFACE void Remove(ECS::EntityID entity) { ECS::RemoveEntity(entity); }