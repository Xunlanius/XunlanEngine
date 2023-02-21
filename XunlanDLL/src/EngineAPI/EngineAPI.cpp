#include "Common.h"
#include "EngineAPI/Script.h"
#include "Platforms/WindowManager.h"
#include "Renderer/Renderer.h"

using namespace Xunlan;

namespace
{
    // Result of DLL loading
    HMODULE g_gameCodeDLL = nullptr;

    // Function pointers
    using GetNames = LPSAFEARRAY (*)();
    using GetCreator = Script::ScriptCreator (*)(const std::string&);
    GetNames g_getNames = nullptr;
    GetCreator g_getCreator = nullptr;

    std::vector<RenderSurface> g_surfaces;
}

EDITOR_INTERFACE uint32_t LoadGameCodeDLL(const char* dllPath)
{
    if (g_gameCodeDLL) return FALSE;

    g_gameCodeDLL = LoadLibraryA(dllPath);
    assert(g_gameCodeDLL);

    // Get the address of the DLL exported functions
    g_getNames = (GetNames)GetProcAddress(g_gameCodeDLL, "IGetScriptNames");
    g_getCreator = (GetCreator)GetProcAddress(g_gameCodeDLL, "IGetScriptCreator");
    assert(g_getNames);
    assert(g_getCreator);

    return (g_gameCodeDLL && g_getNames && g_getCreator) ? TRUE : FALSE;
}
EDITOR_INTERFACE uint32_t UnloadGameCodeDLL()
{
    if (!g_gameCodeDLL) return FALSE;

    int result = FreeLibrary(g_gameCodeDLL);
    assert(result);

    g_gameCodeDLL = nullptr;
    return TRUE;
}

EDITOR_INTERFACE LPSAFEARRAY GetScriptNames()
{
    return (g_gameCodeDLL && g_getNames) ? g_getNames() : nullptr;
}
EDITOR_INTERFACE Script::ScriptCreator GetScriptCreator(const char* scriptName)
{
    return (g_gameCodeDLL && g_getCreator) ? g_getCreator(scriptName) : nullptr;
}

EDITOR_INTERFACE uint32_t CreateRenderSurface(HWND parent, int width, int height)
{
    WindowInitDesc initDesc = {};
    initDesc.parent = parent;
    initDesc.width = width;
    initDesc.height = height;

    RenderSurface renderSurface = {};
    renderSurface.window = WindowManager::CreateWnd(&initDesc);
    renderSurface.renderer = {};

    assert(renderSurface.window.IsValid());

    g_surfaces.push_back(renderSurface);
    return (uint32_t)g_surfaces.size() - 1;
}
EDITOR_INTERFACE void RemoveRenderSurface(uint32_t surfaceID)
{
    assert(surfaceID < g_surfaces.size());
    WindowManager::RemoveWnd(g_surfaces[surfaceID].window);
}
EDITOR_INTERFACE HWND GetWindowHandle(uint32_t surfaceID)
{
    assert(surfaceID < g_surfaces.size());
    return (HWND)WindowManager::GetHandle(g_surfaces[surfaceID].window);
}
EDITOR_INTERFACE void ResizeRenderSurface(uint32_t surfaceID)
{
    assert(surfaceID < g_surfaces.size());
    WindowManager::Resize(g_surfaces[surfaceID].window, 0, 0);
}