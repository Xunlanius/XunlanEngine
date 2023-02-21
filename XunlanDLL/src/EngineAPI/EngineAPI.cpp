#include "Common.h"
#include "EngineAPI/Script.h"
#include "Platforms/Window.h"
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

    std::vector<Graphics::RenderSurface> g_renderSurfaces;
}

EDITOR_INTERFACE uint32 LoadGameCodeDLL(const char* dllPath)
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
EDITOR_INTERFACE uint32 UnloadGameCodeDLL()
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

EDITOR_INTERFACE uint32 CreateRenderSurface(HWND parent, int width, int height)
{
    WindowInitDesc initDesc = {};
    initDesc.parent = parent;
    initDesc.width = width;
    initDesc.height = height;

    Graphics::RenderSurface renderSurface = {};
    renderSurface.windowID = Graphics::Window::Create(&initDesc);
    renderSurface.surfaceID = {};

    assert(ID::IsValid(renderSurface.windowID));

    g_renderSurfaces.push_back(renderSurface);
    return (uint32)g_renderSurfaces.size() - 1;
}
EDITOR_INTERFACE void RemoveRenderSurface(uint32 surfaceID)
{
    assert(surfaceID < g_renderSurfaces.size());
    Graphics::Window::Remove(g_renderSurfaces[surfaceID].windowID);
}
EDITOR_INTERFACE HWND GetWindowHandle(uint32 surfaceID)
{
    assert(surfaceID < g_renderSurfaces.size());
    return (HWND)Graphics::Window::GetHandle(g_renderSurfaces[surfaceID].windowID);
}
EDITOR_INTERFACE void ResizeRenderSurface(uint32 surfaceID)
{
    assert(surfaceID < g_renderSurfaces.size());
    Graphics::Window::Resize(g_renderSurfaces[surfaceID].windowID, 0, 0);
}