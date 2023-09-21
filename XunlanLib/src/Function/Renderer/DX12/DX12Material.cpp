#include "DX12Material.h"
#include "DX12RenderContext.h"
#include "DX12RHI.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    void DX12Material::Apply(Ref<RenderContext> context) const
    {
        Ref<DX12RenderContext> dx12Context = CastTo<DX12RenderContext>(context);
        Ref<DX12Shader> dx12Shader = CastTo<DX12Shader>(m_shader);

        context->SetParam("g_pbr", m_cbPBR);
        context->BindParams(dx12Shader);

        dx12Shader->Apply(context);
    }

    /*void DX12Material::CollectTextureSRVs(Ref<RenderContext> context) const
    {
        std::vector<uint32> textureIndices((size_t)TextureCategory::Count, UINT32_MAX);

        for (uint32 i = 0; i < (uint32)TextureCategory::Count; ++i)
        {
            const CRef<Texture>& texture = m_textureParams[i];
            if (!texture) continue;

            textureIndices[i] = texture->GetHeapIndex();
        }

        BindTextureSRVs(context, textureIndices);
    }

    void DX12Material::BindTextureSRVs(Ref<RenderContext> context, const std::vector<uint32>& textureIndices) const
    {
        Ref<DX12RenderContext> dx12Context = CastTo<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        CB::PerMaterial* perMaterial = (CB::PerMaterial*)m_perMaterial->GetData();
        perMaterial->m_albedoIndex = textureIndices[(uint32)TextureCategory::Albedo];
        perMaterial->m_roughnessIndex = textureIndices[(uint32)TextureCategory::Roughness];
        perMaterial->m_metallicIndex = textureIndices[(uint32)TextureCategory::Metallic];
        perMaterial->m_normalIndex = textureIndices[(uint32)TextureCategory::Normal];
        m_perMaterial->Bind(context);
    }*/
}