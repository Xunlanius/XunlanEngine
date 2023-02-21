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

    struct Script
    {
        ScriptPtr pScript;
    };

    // Called in scripts
    template<class ScriptClass>
    ScriptPtr CreateScript() { return new ScriptClass(); }
    bool RegisterCreator(const std::string& scriptName, ScriptCreator creator);

    void OnUpdate(float deltaTime);

#if defined USE_WITH_EDITOR
    #define EDITOR_EXPORT extern "C" __declspec(dllexport)
#else
    #define EDITOR_EXPORT
#endif
    EDITOR_EXPORT LPSAFEARRAY IGetScriptNames();
    EDITOR_EXPORT ScriptCreator IGetScriptCreator(const std::string& scriptName);

#if !defined SCRIPT_DECL
    #define SCRIPT_DECL(NAME)                               \
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