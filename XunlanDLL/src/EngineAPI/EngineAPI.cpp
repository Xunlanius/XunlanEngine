#include "Common.h"
#include "src/Component/Script.h"
#include "src/Renderer/WindowSystem.h"
#include "src/Renderer/RendererSystem.h"
#include <string>

using namespace Xunlan;

namespace
{
    // Result of DLL loading
    HMODULE g_gameCodeDLL = nullptr;

    // Function pointers
    using GetNames = LPSAFEARRAY(*)();
    using GetCreator = Component::ScriptCreator(*)(const std::string&);
    GetNames g_getNames = nullptr;
    GetCreator g_getCreator = nullptr;

    std::vector<Renderer::RenderSurface> g_renderSurfaces;
}

EDITOR_INTERFACE uint32 LoadGameCodeDLL(const char* dllPath)
{
    if (g_gameCodeDLL) return FALSE;

    g_gameCodeDLL = LoadLibraryA(dllPath);
    assert(g_gameCodeDLL);

    // Get the address of the DLL exported functions
    g_getNames = (GetNames)GetProcAddress(g_gameCodeDLL, "GetRuntimeScriptNames");
    g_getCreator = (GetCreator)GetProcAddress(g_gameCodeDLL, "GetRuntimeScriptCreator");
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
EDITOR_INTERFACE Component::ScriptCreator GetScriptCreator(const char* scriptName)
{
    return (g_gameCodeDLL && g_getCreator) ? g_getCreator(scriptName) : nullptr;
}

EDITOR_INTERFACE uint32 CreateRenderSurface(HWND parent, int width, int height)
{
    Renderer::WindowInitDesc initDesc = {};
    initDesc.parent = parent;
    initDesc.width = width;
    initDesc.height = height;

    Renderer::RenderSurface renderSurface = {};
    renderSurface.windowID = Renderer::WindowSystem::Create(&initDesc);

    assert(IsValid(renderSurface.windowID));

    g_renderSurfaces.push_back(renderSurface);
    return (uint32)g_renderSurfaces.size() - 1;
}
EDITOR_INTERFACE void RemoveRenderSurface(uint32 index)
{
    assert(index < g_renderSurfaces.size());
    Renderer::WindowSystem::Remove(g_renderSurfaces[index].windowID);
}
EDITOR_INTERFACE HWND GetWindowHandle(uint32 index)
{
    assert(index < g_renderSurfaces.size());
    return (HWND)Renderer::WindowSystem::GetHandle(g_renderSurfaces[index].windowID);
}
EDITOR_INTERFACE void ResizeRenderSurface(uint32 index)
{
    assert(index < g_renderSurfaces.size());
    Renderer::WindowSystem::Resize(g_renderSurfaces[index].windowID, 0, 0);
}