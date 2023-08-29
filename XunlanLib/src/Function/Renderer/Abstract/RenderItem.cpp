#include "RenderItem.h"
#include "RHI.h"

namespace Xunlan
{
    RenderItem::RenderItem(Ref<Mesh> mesh)
        : m_mesh(mesh)
    {
        m_perObject = RHI::Instance().CreateCBuffer(CBufferType::PerObject, sizeof(CStruct::PerObject));
    }
    RenderItem::RenderItem(Ref<Mesh> mesh, const std::vector<Ref<Material>>& materials)
        : m_mesh(mesh), m_materials(materials)
    {
        m_perObject = RHI::Instance().CreateCBuffer(CBufferType::PerObject, sizeof(CStruct::PerObject));
    }
}