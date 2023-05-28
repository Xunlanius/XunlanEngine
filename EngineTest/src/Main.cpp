#if defined _WIN64

#include "src/Function/Core/Engine.h"

#include <cassert>
#include <crtdbg.h>
#include <filesystem>

namespace
{
    std::filesystem::path SetCurrDirExecutablePath()
    {
        wchar_t path[MAX_PATH];
        const uint32_t pathLen = GetModuleFileName(0, path, MAX_PATH);
        assert(pathLen > 0);
        if (pathLen == 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER) return {};

        std::filesystem::path p(path);
        std::filesystem::current_path(p.parent_path());

        return std::filesystem::current_path();
    }
}

int main()
{
#if _DEBUG
    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //::_CrtSetBreakAlloc(235);
#endif

    SetCurrDirExecutablePath();

    Xunlan::EngineSystem& engine = Xunlan::Singleton<Xunlan::EngineSystem>::Instance();

    if (!engine.Initialize()) return 0;

    while (engine.IsRunning())
    {
        engine.Tick();
    }
    engine.Shutdown();

    return 0;
}

#endif