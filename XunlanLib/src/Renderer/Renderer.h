#pragma once

#include "Platforms/Window.h"

namespace Xunlan::Graphics
{
    enum class Platform : uint32
    {
        DX12,
    };

    bool Initialize(Platform platform);
    void Shutdown();

    const char* GetEngineShadersBlobPath();
    const char* GetEngineShadersBlobPath(Platform platform);

    struct RenderSurface
    {
        EntityID windowID = ID::INVALID_ID;
        EntityID surfaceID = ID::INVALID_ID;
    };

    class Surface
    {
    public:

        [[nodiscard]] static EntityID Create(EntityID windowID);
        static void Remove(EntityID& surfaceID);
        static void Render(EntityID surfaceID);

        static uint32 GetWidth(EntityID surfaceID);
        static uint32 GetHeight(EntityID surfaceID);
        static void Resize(EntityID surfaceID);
    };
}