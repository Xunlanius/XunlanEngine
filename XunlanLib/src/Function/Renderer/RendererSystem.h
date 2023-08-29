#pragma once

#include "RenderCommon.h"
#include "Abstract/RHI.h"
#include "WindowSystem.h"
#include "RenderPipeline.h"

namespace Xunlan
{
    class RenderSystem final : public Singleton<RenderSystem>
    {
        friend class Singleton<RenderSystem>;

    private:

        RenderSystem() = default;

    public:

        bool Initialize(Platform platform);
        void Shutdown();
        void Render(float deltaTime);

        uint32 GetWidth() const;
        uint32 GetHeight() const;
    };
}