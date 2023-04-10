#pragma once

#include "src/Common/Common.h"
#include <memory>

namespace Xunlan
{
    class GeometryImportSystem;
    class WindowSystem;
    class RenderSystem;

    class RuntimeContext
    {
    public:

        bool Start();
        void Shutdown();

        std::shared_ptr<GeometryImportSystem> m_geometryImportSystem;
        std::shared_ptr<WindowSystem> m_windowSystem;
        std::shared_ptr<RenderSystem> m_renderSystem;
    };

    inline RuntimeContext g_runtimeContext;
}