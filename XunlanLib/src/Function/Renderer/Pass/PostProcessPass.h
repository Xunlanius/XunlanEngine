#pragma once

#include "src/Common/Common.h"
#include "src/Function/Renderer/Texture.h"
#include "src/Function/Renderer/Material.h"
#include "src/Function/Renderer/RenderItem.h"

namespace Xunlan
{
    enum class PostProcessEffect : uint32
    {
        NONE,
        GRAY,
    };

    class PostProcessPass final
    {
    public:

        PostProcessPass();

    public:

        void Render(const Ref<RenderContext>& context, PostProcessEffect effect, const Ref<RenderTarget>& inputRT);

    private:

        void CreateMaterials();
        void CreateCanvas();

    private:

        Ref<RenderItem> m_canvas;
        Ref<Material> m_noneEffect;
    };
}