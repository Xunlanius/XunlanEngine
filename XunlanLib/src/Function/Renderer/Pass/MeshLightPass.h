#pragma once

#include "src/Common/Common.h"
#include "RenderPassBase.h"
#include "src/Function/World/Entity.h"
#include "src/Function/Renderer/Abstract/Texture.h"
#include "src/Function/Renderer/Abstract/RasterizerState.h"
#include "src/Function/Renderer/Abstract/RenderItem.h"

namespace Xunlan
{
    class MeshLightPass final : public RenderPassBase
    {
    public:

        explicit MeshLightPass(uint32 width, uint32 height);

    public:

        void Render(Ref<RenderContext> context);

        Ref<RenderTarget> GetMainRT() const { return m_mainRT; }

    private:

        Ref<Material> CreateMaterial();

        uint32 m_width = 0;
        uint32 m_height = 0;

        Ref<RenderTarget> m_mainRT;
        Ref<RenderItem> m_canvas;
        Ref<Material> m_lighting;
    };
}