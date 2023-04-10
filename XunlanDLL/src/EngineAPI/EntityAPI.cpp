#include "Common.h"
#include "src/ECS/ECS.h"
#include "src/Component/Transformer.h"
#include "src/Component/Script.h"

using namespace Xunlan;

namespace
{
    struct EntityInitInfo
    {
        Component::TransformerInitDesc transformerInitDesc;
        Component::ScriptInitDesc scriptInitDesc;
    };
}

EDITOR_INTERFACE ECS::EntityID Create(const EntityInitInfo& desc)
{
    ECS::EntityID entity = ECS::CreateEntity();

    Component::CreateTransformer(entity, desc.transformerInitDesc);
    Component::AddScript(entity, desc.scriptInitDesc);

    return entity;
}
EDITOR_INTERFACE void Remove(ECS::EntityID entity) { ECS::RemoveEntity(entity); }