#pragma once

#include "RenderCommon.h"
#include "RHI.h"
#include "WindowSystem.h"
#include "Pass/RenderPipeline.h"

namespace Xunlan
{
    class RenderSystem final
    {
    public:

        RenderSystem() = default;
        DISABLE_COPY(RenderSystem)
        DISABLE_MOVE(RenderSystem)

    public:

        bool Initialize(Platform platform, const RenderSystemInitDesc& initDesc);
        void Shutdown();
        void Render(float deltaTime);

        uint32 GetWidth() const;
        uint32 GetHeight() const;

    private:

        std::shared_ptr<WindowSystem> m_windowSystem;
    };
}