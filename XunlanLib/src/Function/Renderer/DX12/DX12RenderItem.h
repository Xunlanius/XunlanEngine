#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/Abstract/RenderItem.h"

namespace Xunlan::DX12
{
    class DX12RenderItem final : public RenderItem
    {
    public:

        explicit DX12RenderItem(Ref<Mesh> mesh)
            : RenderItem(mesh) {}
        explicit DX12RenderItem(Ref<Mesh> mesh, const std::vector<Ref<Material>>& materials)
            : RenderItem(mesh, materials) {}

    public:

        virtual void Render(Ref<RenderContext> context) const override;
        virtual void Render(Ref<RenderContext> context, CRef<Material> overrideMaterial) const override;

    private:

        void DrawMesh(Ref<RenderContext> context, CRef<Material> overrideMaterial) const;
    };
}