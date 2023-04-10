#include "Script.h"
#include <unordered_map>

namespace Xunlan
{
    namespace
    {
        using RegisterMap = std::unordered_map<std::string, ScriptCreator>;
        RegisterMap& GetRegisterMap()
        {
            static RegisterMap map = RegisterMap();
            return map;
        }
    }

    void AddScript(ECS::EntityID entity, const ScriptInitDesc& initDesc)
    {
        Script script = {};
        script.pScript = initDesc.scriptCreator ? initDesc.scriptCreator() : nullptr;
        
        if (script.pScript) ECS::AddComponent<Script>(entity, script);
    }

    bool RegisterCreator(const std::string& scriptName, ScriptCreator creator)
    {
        bool result = GetRegisterMap().insert({ scriptName, creator }).second;
        assert(result);
        return result;
    }

    LPSAFEARRAY GetRuntimeScriptNames()
    {
        const size_t len = GetRegisterMap().size();
        if (len == 0) return nullptr;

        CComSafeArray<BSTR> names((ULONG)len);
        size_t index = 0;

        for (const auto& [name, creator] : GetRegisterMap())
        {
            names.SetAt((LONG)index, A2BSTR_EX(name.c_str()), false);
            ++index;
        }

        return names.Detach();
    }
    ScriptCreator GetRuntimeScriptCreator(const std::string& scriptName)
    {
        auto find = GetRegisterMap().find(scriptName);
        assert(find != GetRegisterMap().end());
        return find->second;
    }
}