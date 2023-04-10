#pragma once

#include "src/Common/Common.h"
#include "src/Function/World/ECS/ECS.h"

namespace Xunlan
{
    // Should be inherited by the instance of game script
    class RuntimeScript
    {
    public:

        virtual void Initialize() {}
        virtual void OnUpdate(float deltaTime) {}
    };

    using ScriptPtr = RuntimeScript*;
    using ScriptCreator = ScriptPtr(*)();
    
    // Called in scripts
    template<class ScriptClass>
    ScriptPtr CreateScript() { return new ScriptClass(); }
    bool RegisterCreator(const std::string& scriptName, ScriptCreator creator);

    struct ScriptInitDesc final
    {
        ScriptCreator scriptCreator;
    };

    void AddScript(ECS::EntityID entity, const ScriptInitDesc& initDesc);

    struct Script final
    {
        ScriptPtr pScript;
    };

    struct ScriptSystem : public ECS::ISystem
    {
        void Initialize()
        {
            for (ECS::Entity entity : m_entityIDs)
            {
                auto [script] = m_manager.GetComponent<Script>(entity);
                script.pScript->Initialize();
            }
        }

        void OnUpdate(float deltaTime)
        {
            for (ECS::Entity entity : m_entityIDs)
            {
                auto [script] = m_manager.GetComponent<Script>(entity);
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
    class NAME : public Xunlan::Component::RuntimeScript    \
    {                                                       \
    public:                                                 \
                                                            \
        virtual void Initialize() override;                 \
        virtual void OnUpdate(float deltaTime) override;    \
    };
#endif

#if !defined SCRIPT_REGISTER
#define SCRIPT_REGISTER(NAME)   \
    const bool g_register_##NAME = Xunlan::Component::RegisterCreator(#NAME, Xunlan::Component::CreateScript<NAME>);
#endif
}