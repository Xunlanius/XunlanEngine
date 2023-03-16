#pragma once

#include "Common/Common.h"

namespace Xunlan::Script
{
    // Should be inherited by the instance of game script
    class RuntimeScript
    {
    public:

        virtual void Initialize() {}
        virtual void OnUpdate(float deltaTime) {}
    };

    using ScriptPtr = RuntimeScript*;
    using ScriptCreator = ScriptPtr (*)();

    // Called in scripts
    template<class ScriptClass>
    ScriptPtr CreateScript() { return new ScriptClass(); }
    bool RegisterCreator(const std::string& scriptName, ScriptCreator creator);

    struct Script
    {
        ScriptPtr pScript;
    };

    struct ScriptSystem : public ECS::ISystem
    {
        void Initialize()
        {
            for (ECS::EntityID entity : m_entities)
            {
                Script& script = ECS::GetComponent<Script>(entity);
                script.pScript->Initialize();
            }
        }

        void OnUpdate(float deltaTime)
        {
            for (ECS::EntityID entity : m_entities)
            {
                Script& script = ECS::GetComponent<Script>(entity);
                script.pScript->OnUpdate(deltaTime);
            }
        }
    };

#if defined USE_WITH_EDITOR
#define EDITOR_EXPORT extern "C" __declspec(dllexport)
#else
#define EDITOR_EXPORT
#endif

    EDITOR_EXPORT LPSAFEARRAY GetRuntimeScriptNames();
    EDITOR_EXPORT ScriptCreator GetRuntimeScriptCreator(const std::string& scriptName);

#if !defined SCRIPT_DECL
#define SCRIPT_DECL(NAME)                                   \
    class NAME : public Xunlan::Script::RuntimeScript       \
    {                                                       \
    public:                                                 \
                                                            \
        virtual void Initialize() override;                 \
        virtual void OnUpdate(float deltaTime) override;    \
    };
#endif

#if !defined SCRIPT_REGISTER
#define SCRIPT_REGISTER(NAME)   \
    const bool g_register_##NAME = Xunlan::Script::RegisterCreator(#NAME, Xunlan::Script::CreateScript<NAME>);
#endif
}