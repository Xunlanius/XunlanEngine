#pragma once

#include "src/Common/Common.h"
#include "Pass/ShadowPass.h"
#include "Pass/GPass.h"
#include "Pass/RSMPass.h"
#include "Pass/LightingPass.h"
#include "Pass/PostProcessPass.h"
#include "src/Function/World/Component/Transformer.h"

namespace Xunlan
{
    class RenderPipeline final : public Singleton<RenderPipeline>
    {
        friend class Singleton<RenderPipeline>;

    private:

        RenderPipeline() = default;

    public:

        void Initialize(uint32 width, uint32 height);
        void Shutdown();

        void RenderForward();

    private:

        void UpdateItems();
        void ClearItems();

        void UpdateCBufferPerFrame();
        void UpdateCBufferLights();
        static void UpdateCBufferPerObject(const TransformerComponent& transformer, Ref<CBuffer> cBufferPerObject);

    private:

        Ref<CBuffer> m_cbPerFrame;
        Ref<CBuffer> m_cbLights;

        std::unique_ptr<ShadowPass> m_shadowPass;
        std::unique_ptr<GPass> m_gPass;
        std::unique_ptr<RSMPass> m_rsmPass;
        std::unique_ptr<LightingPass> m_lightingPass;
        std::unique_ptr<PostProcessPass> m_postProcessPass;

        std::vector<Ref<RenderItem>> m_items;
        std::vector<Ref<RenderItem>> m_castShadowItems;
    };
}