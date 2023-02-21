#define TEST_ENTITY 0
#define TEST_WINDOW 0
#define TEST_WORLD 1

#if TEST_ENTITY
    #include "Test/TestEntity.h"
#elif TEST_WINDOW
    #include "Test/TestWindow.h"
#elif TEST_WORLD
    #include "Test/TestWorld.h"
#endif

#if defined _WIN64

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    ITest* test = new TestWorld();

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