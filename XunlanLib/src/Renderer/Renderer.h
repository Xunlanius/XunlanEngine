#pragma once

#include "Platforms/WindowManager.h"

namespace Xunlan
{
    class Renderer
    {
    };

    struct RenderSurface
    {
        Window window;
        Renderer renderer;
    };
}