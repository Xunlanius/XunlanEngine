#pragma once

#include "src/Common/Common.h"
#include "ShadowPass.h"
#include "GPass.h"
#include "MeshLightPass.h"
#include "PostProcessPass.h"

namespace Xunlan
{
    class RenderPipeline final
    {
        friend class Singleton<RenderPipeline>;

    private:

        RenderPipeline() = default;
        DISABLE_COPY(RenderPipeline)
        DISABLE_MOVE(RenderPipeline)
        ~RenderPipeline() = default;

    public:

        void Initialize(uint32 width, uint32 height);
        void Shutdown();

        void RenderForward();

    private:

        std::unique_ptr<ShadowPass> m_shadowPass;
        std::unique_ptr<GPass> m_gPass;
        std::unique_ptr<MeshLightPass> m_meshLightPass;
        std::unique_ptr<PostProcessPass> m_postProcessPass;
    };
}