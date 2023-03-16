#pragma once

#include "Platforms/PlatformsType.h"

namespace Xunlan
{
    class Window : public IDModel
    {
    public:

        constexpr Window()
            : IDModel(ID::ms_invalidID) {}
        constexpr explicit Window(ID::id_type id)
            : IDModel(id) {}
    };

    class WindowManager
    {
    public:

        static Window CreateWnd(const WindowInitDesc* const pDesc = nullptr);
        static void RemoveWnd(Window window);

    public:

        static void* GetHandle(Window window);

        static void SetCaption(Window window, const wchar_t* caption);

        static uint32_t GetWidth(Window window);
        static uint32_t GetHeight(Window window);

        static DirectX::XMUINT4 GetSize(Window window);
        static void Resize(Window window, uint32_t width, uint32_t height);

        static bool IsFullScreen(Window window);
        static void SetFullScreen(Window window, bool isFullScreen);

        static bool IsClosed(Window window);
    };
}