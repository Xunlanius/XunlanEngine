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
        GraphicsAPI g_platformAPI = {};

        bool SetPlatform(Platform platform)
        {
            switch (platform)
            {
            case Platform::DX12:
                DX12::GetGraphicsAPI(g_platformAPI);
                break;

            default:
                return false;
            }

            assert(platform == g_platformAPI.platform);
            return true;
        }
    }

    bool Initialize(Platform platform) { return SetPlatform(platform) && g_platformAPI.initialize(); }
    void Shutdown() { g_platformAPI.shutdown(); }

    const char* GetEngineShadersBlobPath() { return g_engineShaderPaths[(uint32)g_platformAPI.platform]; }
    const char* GetEngineShadersBlobPath(Platform platform) { return g_engineShaderPaths[(uint32)platform]; }

    EntityID Surface::Create(EntityID windowID) { return g_platformAPI.surface.create(windowID); }
    void Surface::Remove(EntityID& surfaceID) { g_platformAPI.surface.remove(surfaceID); }
    void Surface::Render(EntityID surfaceID) { g_platformAPI.surface.render(surfaceID); }

    uint32 Surface::GetWidth(EntityID surfaceID) { return g_platformAPI.surface.getWidth(surfaceID); }
    uint32 Surface::GetHeight(EntityID surfaceID) { return g_platformAPI.surface.getHeight(surfaceID); }

    void Surface::Resize(EntityID surfaceID) { g_platformAPI.surface.resize(surfaceID); }
}