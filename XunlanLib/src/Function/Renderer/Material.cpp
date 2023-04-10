#include "Material.h"
#include "RHI.h"

namespace Xunlan
{
    Material::Material(const std::string& name, MaterialType type, const ShaderList& shaderList)
        : m_name(name), m_type(type)
    {
        m_VS = shaderList.m_VS;
        m_PS = shaderList.m_PS;

        RHI& rhi = RHI::Instance();

        m_rasterizerState = rhi.CreateRasterizerState({});
        m_depthStencilState = rhi.CreateDepthStencilState();
        m_perMaterial = rhi.CreateConstantBuffer(CBufferType::PER_MATERIAL, 1024);
        m_textureIndices = rhi.CreateConstantBuffer(CBufferType::TEXTURE_INDICES, sizeof(CBufferTextureIndices));
    }
}