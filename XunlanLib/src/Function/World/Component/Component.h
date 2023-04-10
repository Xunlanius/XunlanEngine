#pragma once

#include "src/Function/World/ECS/ECS.h"
#include "Transformer.h"
#include "NativeScript.h"
#include "Camera.h"
#include "MeshRenderComponent.h"

namespace Xunlan
{
    inline void RegisterECS()
    {
        ECS::ECSManager& instance = Singleton<ECS::ECSManager>::Instance();
        instance.RegisterSystem<ScriptSystem, NativeScriptComponent>();
        instance.RegisterSystem<CameraSystem, TransformerComponent, CameraComponent>();
        instance.RegisterSystem<MeshRenderSystem, TransformerComponent>();
    }
}