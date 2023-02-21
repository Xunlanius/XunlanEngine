#include "Script.h"

namespace Xunlan::Script
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

    bool RegisterCreator(const std::string& scriptName, ScriptCreator creator)
    {
        bool result = GetRegisterMap().insert({ scriptName, creator }).second;
        assert(result);
        return result;
    }

    void OnUpdate(float deltaTime)
    {
        Queryer queryer(World::GetWorld());
        std::vector<EntityID> entities = queryer.Query<Script>();
        for (EntityID entity : entities)
        {
            queryer.GetComponent<Script>(entity).pScript->OnUpdate(deltaTime);
        }
    }

    LPSAFEARRAY IGetScriptNames()
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
    ScriptCreator IGetScriptCreator(const std::string& scriptName)
    {
        auto find = GetRegisterMap().find(scriptName);
        assert(find != GetRegisterMap().end());
        return find->second;
    }
}