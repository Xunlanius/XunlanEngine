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

EDITOR_INTERFACE ID::id_type Create(const EntityInitInfo& info)
{
    assert(&info);

    Transformer::Transformer transformer = info.transformer.ToEngineInfo();
    Script::Script script = info.script.ToEngineInfo();

    Command cmd(World::GetWorld());
    EntityID entity = -1;

    if (script.pScript)
    {
        entity = cmd.Create<Transformer::Transformer, Script::Script>(
            info.transformer.ToEngineInfo(),
            info.script.ToEngineInfo());
    }
    else
    {
        entity = cmd.Create<Transformer::Transformer>(
            info.transformer.ToEngineInfo());
    }
    return entity;
}

EDITOR_INTERFACE void Remove(ID::id_type id)
{
    Command cmd(World::GetWorld());
    cmd.Remove(id);
}