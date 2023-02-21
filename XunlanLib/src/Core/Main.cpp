#if defined _WIN64

#ifndef WIN32_MEAN_AND_LEAN
    #define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <crtdbg.h>

#if !defined USE_WITH_EDITOR

extern bool EngineInitialize();
extern void EngineOnTick();
extern void EngineShutdown();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

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