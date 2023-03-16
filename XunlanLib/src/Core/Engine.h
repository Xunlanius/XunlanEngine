#pragma once

#include "EngineAPI/Transformer.h"
#include "EngineAPI/Script.h"

namespace Xunlan
{
    inline void RegisterECS()
    {
        ECS::RegisterComponent<Transformer::Transformer>();
        ECS::RegisterComponent<Script::Script>();

        ECS::RegisterSystem<Script::ScriptSystem, Script::Script>();
    }
}