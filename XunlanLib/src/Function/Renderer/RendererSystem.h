#pragma once

#include "RenderCommon.h"
#include "Abstract/RHI.h"
#include "WindowSystem.h"
#include "Pass/RenderPipeline.h"

namespace Xunlan
{
    class RenderSystem final
    {
        friend class Singleton<RenderSystem>;

    private:

        RenderSystem() = default;
        DISABLE_COPY(RenderSystem)
        DISABLE_MOVE(RenderSystem)
        ~RenderSystem() = default;

    public:

        bool Initialize(Platform platform);
        void Shutdown();
        void Render(float deltaTime);

        uint32 GetWidth() const;
        uint32 GetHeight() const;
    };
}