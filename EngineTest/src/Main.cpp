#define TEST_ENTITY 0
#define TEST_ECS 0
#define TEST_RENDERER 1

#if TEST_ENTITY
    #include "TestEntity.h"
#elif TEST_ECS
    #include "TestECS.h"
#elif TEST_RENDERER
    #include "TestRenderer.h"
#endif

#include <filesystem>

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

#if defined _WIN64

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    SetCurrDirExecutablePath();

    ITest* test = new TestRenderer();

    if (test->Initialize())
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

            test->Run();
        }
}

    test->Shutdown();
    }

    delete test;

    return 0;
}

#else

int main()
{
#if _DEBUG
    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    ITest* test = new TestWindow();

    if (test->Initialize())
    {
        test->Run();
        test->Shutdown();
    }

    delete test;

    return 0;
}

#endif