#pragma once

#include "Renderer.h"

namespace Xunlan::Graphics
{

    struct GraphicsAPI
    {
        Platform platform = (Platform)-1;

        using InitFunc = bool(*)();
        using ShutdownFunc = void(*)();

        InitFunc initialize;
        ShutdownFunc shutdown;

        struct
        {
            using CreateFunc = EntityID(*)(EntityID windowID);
            using RemoveFunc = void(*)(EntityID& surfaceID);
            using RenderFunc = void(*)(EntityID surfaceID);
            using GetWidthFunc = uint32(*)(EntityID surfaceID);
            using GetHeightFunc = uint32(*)(EntityID surfaceID);
            using ResizeFunc = void(*)(EntityID surfaceID);

            CreateFunc create;
            RemoveFunc remove;
            RenderFunc render;

            GetWidthFunc getWidth;
            GetHeightFunc getHeight;
            ResizeFunc resize;
        } surface;
    };
}