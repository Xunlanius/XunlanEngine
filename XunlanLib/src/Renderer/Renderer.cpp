#include "Renderer.h"
#include "Renderer/GraphicsAPI.h"
#include "Renderer/DX12/DX12API.h"

namespace Xunlan::Graphics
{
    namespace
    {
        // relative to output directory
        constexpr const char* g_engineShaderPaths[] =
        {
            "./Shaders/DX12Shaders.bin"
        };
        GraphicsAPI g_graphicsAPI = {};

        bool SetPlatform(Platform platform)
        {
            switch (platform)
            {
            case Platform::DX12:
                DX12::GetGraphicsAPI(g_graphicsAPI);
                break;

            default:
                return false;
            }

            assert(platform == g_graphicsAPI.platform);
            return true;
        }
    }

    bool Initialize(Platform platform) { return SetPlatform(platform) && g_graphicsAPI.initialize(); }
    void Shutdown() { g_graphicsAPI.shutdown(); }

    const char* GetEngineShadersBlobPath() { return g_engineShaderPaths[(uint32)g_graphicsAPI.platform]; }
    const char* GetEngineShadersBlobPath(Platform platform) { return g_engineShaderPaths[(uint32)platform]; }

    ID Surface::Create(ID windowID) { return g_graphicsAPI.surface.create(windowID); }
    void Surface::Remove(ID& surfaceID) { g_graphicsAPI.surface.remove(surfaceID); }
    void Surface::Render(ID surfaceID) { g_graphicsAPI.surface.render(surfaceID); }

    uint32 Surface::GetWidth(ID surfaceID) { return g_graphicsAPI.surface.getWidth(surfaceID); }
    uint32 Surface::GetHeight(ID surfaceID) { return g_graphicsAPI.surface.getHeight(surfaceID); }

    void Surface::Resize(ID surfaceID) { g_graphicsAPI.surface.resize(surfaceID); }
}