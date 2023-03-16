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
        ID windowID = INVALID_ID;
        ID surfaceID = INVALID_ID;
    };

    class Surface
    {
    public:

        [[nodiscard]] static ID Create(ID windowID);
        static void Remove(ID& surfaceID);
        static void Render(ID surfaceID);

        static uint32 GetWidth(ID surfaceID);
        static uint32 GetHeight(ID surfaceID);
        static void Resize(ID surfaceID);
    };
}