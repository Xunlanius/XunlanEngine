#pragma once

#include "src/Common/Common.h"
#include "Mesh.h"
#include "CBuffer.h"
#include "Material.h"

namespace Xunlan
{
    class RenderItem
    {
    protected:

        explicit RenderItem(Ref<Mesh> mesh);
        explicit RenderItem(Ref<Mesh> mesh, const std::vector<Ref<Material>>& materials);
        DISABLE_COPY(RenderItem)
        DISABLE_MOVE(RenderItem)
        virtual ~RenderItem() = default;

    public:

        Ref<CBuffer> GetPerObject() const { return m_perObject; }
        Ref<Material> GetMaterial(uint32 index) const { assert(index < m_materials.size()); return m_materials[index]; }

        virtual void Render(Ref<RenderContext> context) const = 0;
        virtual void Render(Ref<RenderContext> context, CRef<Material> overrideMaterial) const = 0;

    protected:

        Ref<Mesh> m_mesh;
        Ref<CBuffer> m_perObject;
        std::vector<Ref<Material>> m_materials;
    };
}