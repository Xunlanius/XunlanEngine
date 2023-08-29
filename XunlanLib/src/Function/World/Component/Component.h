#pragma once

#include "src/Function/World/ECS/World.h"
#include "Transformer.h"
#include "NativeScript.h"
#include "Camera.h"
#include "MeshRender.h"

namespace Xunlan
{
    inline void RegisterECS()
    {
        ECS::World& world = ECS::World::Instance();
        world.RegisterSystem<ScriptSystem, NativeScriptComponent>();
        world.RegisterSystem<CameraSystem, TransformerComponent, CameraComponent>();
        world.RegisterSystem<MeshRenderSystem, MeshRenderComponent, TransformerComponent>();
    }
}