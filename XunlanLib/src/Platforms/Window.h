#pragma once

#include "Platforms/PlatformsType.h"
#include "Utilities/MathTypes.h"

namespace Xunlan::Graphics
{
    class Window
    {
    public:

        [[nodiscard]] static EntityID Create(const WindowInitDesc* const pDesc = nullptr);
        static void Remove(EntityID& windowID);

    public:

        static void* GetHandle(EntityID windowID);

        static void SetCaption(EntityID windowID, const wchar_t* caption);

        static uint32 GetWidth(EntityID windowID);
        static uint32 GetHeight(EntityID windowID);

        static Math::UVector4 GetSize(EntityID windowID);
        static void Resize(EntityID windowID, uint32 width, uint32 height);

        static bool IsFullScreen(EntityID windowID);
        static void SetFullScreen(EntityID windowID, bool isFullScreen);

        [[nodiscard]] static bool IsClosed(EntityID windowID);
    };
}