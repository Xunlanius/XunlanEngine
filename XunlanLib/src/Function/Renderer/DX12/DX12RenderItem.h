#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/RenderItem.h"

namespace Xunlan::DX12
{
    class DX12RenderItem final : public RenderItem
    {
    public:

        explicit DX12RenderItem(const Ref<Mesh>& mesh)
            : RenderItem(mesh) {}
        explicit DX12RenderItem(const Ref<Mesh>& mesh, const std::vector<Ref<Material>>& materials)
            : RenderItem(mesh, materials) {}

    public:

        virtual void Render(const Ref<RenderContext>& context) const override;
        virtual void Render(const Ref<RenderContext>& context, const CRef<Material>& overrideMaterial) const override;

    private:

        void DrawMesh(const Ref<RenderContext>& context, const CRef<Material>& overrideMaterial) const;
    };
}