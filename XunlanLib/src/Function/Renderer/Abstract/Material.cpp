#include "Material.h"
#include "RHI.h"

namespace Xunlan
{
    Material::Material(Ref<Shader> shader) :
        m_shader(shader)
    {
        m_cbPBR = RHI::Instance().CreateCBuffer<CB::PBR>();
    }
}