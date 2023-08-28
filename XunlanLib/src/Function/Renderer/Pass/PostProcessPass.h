#pragma once

#include "src/Common/Common.h"
#include "RenderPassBase.h"
#include "src/Function/Renderer/Abstract/Texture.h"
#include "src/Function/Renderer/Abstract/Material.h"
#include "src/Function/Renderer/Abstract/RenderItem.h"

namespace Xunlan
{
    enum class PostProcessEffect : uint32
    {
        NONE,
        GRAY,
    };

    class PostProcessPass final : public RenderPassBase
    {
    public:

        PostProcessPass();

    public:

        void Render(Ref<RenderContext> context, PostProcessEffect effect, CRef<RenderTarget> inputRT);

    private:

        void CreateMaterials();

    private:

        Ref<RenderItem> m_canvas;
        Ref<Material> m_noneEffect;
    };
}