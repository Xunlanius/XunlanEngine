#pragma once

#include "RenderCommon.h"
#include <vector>
#include <functional>

namespace Xunlan
{
    class WindowSystem final : public Singleton<WindowSystem>
    {
        friend class Singleton<WindowSystem>;

    private:

        WindowSystem() = default;

    public:

        bool Initialize(const WindowInitDesc& initDesc);
        void Shutdown();

        void* GetHandle() const;
        void SetCaption(const wchar_t* caption);

        uint32 GetWidth() const;
        uint32 GetHeight() const;

    public:

        using OnResizeFunc = std::function<void(uint32 width, uint32 height)>;
        using OnMoveFunc = std::function<void(int x, int y)>;
        using OnClosedFunc = std::function<void()>;

        auto RegisterResizeFunc(OnResizeFunc func) { return m_onResizeFuncs.emplace(m_onResizeFuncs.end(), func); }
        auto RegisterMoveFunc(OnResizeFunc func) { return m_onMoveFuncs.emplace(m_onMoveFuncs.end(), func); }
        auto RegisterClosedFunc(OnClosedFunc func) { return m_onClosedFuncs.emplace(m_onClosedFuncs.end(), func); }

    private:

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        template<typename Callback, typename... Args>
        static void Dispatch(HWND hwnd, Callback callback, Args... args)
        {
            WindowSystem* windowSystem = (WindowSystem*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (windowSystem) (windowSystem->*callback)(args...);
        }

        void OnResized(uint32 width, uint32 height);
        void OnMoved(int x, int y);
        void OnClosed();

    private:

        WindowInfo m_windowInfo;

        std::vector<OnResizeFunc> m_onResizeFuncs;
        std::vector<OnMoveFunc> m_onMoveFuncs;
        std::vector<OnClosedFunc> m_onClosedFuncs;
    };
}