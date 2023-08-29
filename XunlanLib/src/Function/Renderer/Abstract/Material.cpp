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
        m_perMaterial = rhi.CreateCBuffer(CBufferType::PerMaterial, sizeof(CStruct::PerMaterial));

        SetAlbedo({ 1.0f, 1.0f, 1.0f, 1.0f });
        SetRoughness(0.5f);
        SetMetallic(0.5f);
    }

    Math::float4 Material::GetAlbedo() const
    {
        CStruct::PerMaterial* perMaterial = (CStruct::PerMaterial*)m_perMaterial->GetData();
        return perMaterial->m_albedo;
    }

    float Material::GetRoughness() const
    {
        CStruct::PerMaterial* perMaterial = (CStruct::PerMaterial*)m_perMaterial->GetData();
        return perMaterial->m_roughness;
    }

    float Material::GetMetallic() const
    {
        CStruct::PerMaterial* perMaterial = (CStruct::PerMaterial*)m_perMaterial->GetData();
        return perMaterial->m_metallic;
    }

    CRef<Texture> Material::GetTexture(TextureCategory category) const
    {
        assert(category < TextureCategory::Count);
        return m_textureParams[(uint32)category];
    }

    void Material::SetAlbedo(const Math::float4& value)
    {
        CStruct::PerMaterial* perMaterial = (CStruct::PerMaterial*)m_perMaterial->GetData();
        perMaterial->m_albedo = value;
    }

    void Material::SetRoughness(float value)
    {
        CStruct::PerMaterial* perMaterial = (CStruct::PerMaterial*)m_perMaterial->GetData();
        perMaterial->m_roughness = value;
    }

    void Material::SetMetallic(float value)
    {
        CStruct::PerMaterial* perMaterial = (CStruct::PerMaterial*)m_perMaterial->GetData();
        perMaterial->m_metallic = value;
    }

    void Material::SetTexture(TextureCategory category, CRef<Texture> value)
    {
        assert(value);
        m_textureParams[(uint32)category] = value;
    }
}