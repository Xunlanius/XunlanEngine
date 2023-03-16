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
            using CreateFunc = ID(*)(ID windowID);
            using RemoveFunc = void(*)(ID& surfaceID);
            using RenderFunc = void(*)(ID surfaceID);
            using GetWidthFunc = uint32(*)(ID surfaceID);
            using GetHeightFunc = uint32(*)(ID surfaceID);
            using ResizeFunc = void(*)(ID surfaceID);

            CreateFunc create;
            RemoveFunc remove;
            RenderFunc render;

            GetWidthFunc getWidth;
            GetHeightFunc getHeight;
            ResizeFunc resize;
        } surface;
    };
}