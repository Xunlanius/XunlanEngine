#if defined _WIN64

#ifndef WIN32_MEAN_AND_LEAN
    #define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <crtdbg.h>
#include <cassert>
#include <filesystem>
#include "Common/DataTypes.h"

namespace
{
    std::filesystem::path SetCurrDirExecutablePath()
    {
        wchar_t path[MAX_PATH];
        const uint32 pathLen = GetModuleFileName(0, path, MAX_PATH);
        assert(pathLen > 0);
        if (pathLen == 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER) return {};

        std::filesystem::path p(path);
        std::filesystem::current_path(p.parent_path());

        return std::filesystem::current_path();
    }
}

#if !defined USE_WITH_EDITOR

extern bool EngineInitialize();
extern void EngineOnTick();
extern void EngineShutdown();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    SetCurrDirExecutablePath();

    if (EngineInitialize())
    {
        MSG msg = {};
        bool isRunning = true;

        while (isRunning)
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT) isRunning = false;
            }

            EngineOnTick();
        }

        EngineShutdown();
    }

    return 0;
}

#endif

#endif