#include "RenderItem.h"
#include "RHI.h"

namespace Xunlan
{
    RenderItem::RenderItem(const Ref<Mesh>& mesh)
        : m_mesh(mesh)
    {
        m_perObject = RHI::Instance().CreateCBuffer(CBufferType::PerObject, sizeof(CBufferPerObject));
    }
    RenderItem::RenderItem(const Ref<Mesh>& mesh, const std::vector<Ref<Material>>& materials)
        : m_mesh(mesh), m_materials(materials)
    {
        m_perObject = RHI::Instance().CreateCBuffer(CBufferType::PerObject, sizeof(CBufferPerObject));
    }
}