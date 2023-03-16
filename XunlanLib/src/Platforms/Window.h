#pragma once

#include "Platforms/PlatformsType.h"
#include "Utility/MathTypes.h"

namespace Xunlan::Graphics
{
    class Window
    {
    public:

        [[nodiscard]] static ID Create(const WindowInitDesc* const pDesc = nullptr);
        static void Remove(ID& windowID);

    public:

        static void* GetHandle(ID windowID);

        static void SetCaption(ID windowID, const wchar_t* caption);

        static uint32 GetWidth(ID windowID);
        static uint32 GetHeight(ID windowID);

        static Math::U32Vector4 GetSize(ID windowID);
        static void Resize(ID windowID, uint32 width, uint32 height);

        static bool IsFullScreen(ID windowID);
        static void SetFullScreen(ID windowID, bool isFullScreen);

        static bool IsClosed(ID windowID);
    };
}