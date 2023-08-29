#include "DX12Material.h"
#include "DX12RenderContext.h"
#include "Texture/DX12ImageTexture.h"
#include "Texture/DX12RenderTarget.h"
#include "DX12RootParameter.h"
#include "DX12RHI.h"
#include "DX12Helper.h"
#include <numeric>

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    bool operator==(const DX12PSO& lhs, const DX12PSO& rhs)
    {
        return lhs.m_VS == rhs.m_VS &&
            lhs.m_PS == rhs.m_PS &&
            *lhs.m_rasterizerState == *rhs.m_rasterizerState &&
            *lhs.m_depthStencilState == *rhs.m_depthStencilState &&
            lhs.m_rtFormats == rhs.m_rtFormats &&
            lhs.m_dsFormat == rhs.m_dsFormat;
    }

    size_t DX12PSO::Hash::operator()(const DX12PSO& key) const
    {
        size_t result = 0;
        result += std::hash<CRef<DX12Shader>>()(key.m_VS);
        result += std::hash<CRef<DX12Shader>>()(key.m_PS);
        result += DX12RasterizerState::Hash()(*key.m_rasterizerState);
        result += DX12DepthStencilState::Hash()(*key.m_depthStencilState);
        result += std::accumulate(key.m_rtFormats.begin(), key.m_rtFormats.end(), 0);
        result += key.m_dsFormat;
        return result;
    }

    void DX12Material::Apply(Ref<RenderContext> context) const
    {
        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        // Set PSO
        ComPtr<ID3D12PipelineState> pso = GetPSO();
        cmdList->SetPipelineState(pso.Get());

        // Bind material params
        CollectTextureSRVs(context);
    }

    void DX12Material::CollectTextureSRVs(Ref<RenderContext> context) const
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

        CStruct::PerMaterial* perMaterial = (CStruct::PerMaterial*)m_perMaterial->GetData();
        perMaterial->m_albedoIndex = textureIndices[(uint32)TextureCategory::Albedo];
        perMaterial->m_roughnessIndex = textureIndices[(uint32)TextureCategory::Roughness];
        perMaterial->m_metallicIndex = textureIndices[(uint32)TextureCategory::Metallic];
        perMaterial->m_normalIndex = textureIndices[(uint32)TextureCategory::Normal];
        m_perMaterial->Bind(context);
    }

    ComPtr<ID3D12PipelineState> DX12Material::GetPSO() const
    {
        DX12RHI& rhi = DX12RHI::Instance();
        CRef<DX12Shader> vs = CastTo<const DX12Shader>(m_VS);
        CRef<DX12Shader> ps = CastTo<const DX12Shader>(m_PS);
        CRef<DX12RasterizerState> rasterizerState = CastTo<const DX12RasterizerState>(m_rasterizerState);
        CRef<DX12DepthStencilState> depthStencilState = CastTo<const DX12DepthStencilState>(m_depthStencilState);

        DX12PSO psoDesc = {};
        psoDesc.m_VS = vs;
        psoDesc.m_PS = ps;
        psoDesc.m_rasterizerState = rasterizerState;
        psoDesc.m_depthStencilState = depthStencilState;
        psoDesc.m_rtFormats = rhi.GetRTFormats();
        psoDesc.m_dsFormat = rhi.GetDSFormat();

        auto& psoContainer = rhi.GetPSOContainer();

        auto it = psoContainer.find(psoDesc);
        if (it != psoContainer.end())
        {
            return it->second;
        }
        else
        {
            auto [newOne, succeed] = psoContainer.emplace(psoDesc, CreatePSO(psoDesc));
            return newOne->second;
        }
    }

    ComPtr<ID3D12PipelineState> DX12Material::CreatePSO(const DX12PSO& psoDesc) const
    {
        DX12RHI& rhi = DX12RHI::Instance();
        Device& device = rhi.GetDevice();

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = rhi.GetDefaultRootSig();
        if (psoDesc.m_VS) desc.VS = psoDesc.m_VS->GetByteCode();
        if (psoDesc.m_PS) desc.PS = psoDesc.m_PS->GetByteCode();
        desc.BlendState = Helper::Blend::DISABLED;
        desc.SampleMask = UINT32_MAX; // It is neccessary. Without it, nothing will be rendered.
        desc.RasterizerState = psoDesc.m_rasterizerState->GetDX12Desc();
        desc.DepthStencilState = psoDesc.m_depthStencilState->GetDX12Desc();
        //desc.InputLayout = rhi.GetInputLayout();
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets = std::min((uint32)psoDesc.m_rtFormats.size(), 8u);
        if (desc.NumRenderTargets > 0)
        {
            const size_t byteSize = desc.NumRenderTargets * sizeof(DXGI_FORMAT);
            std::memcpy(desc.RTVFormats, psoDesc.m_rtFormats.data(), byteSize);
        }
        desc.DSVFormat = psoDesc.m_dsFormat;
        desc.SampleDesc = { 1, 0 };

        ComPtr<ID3D12PipelineState> pso;
        Check(device.CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso)));
        NAME_OBJECT_INDEX(pso, L"PSO - key: ", DX12PSO::Hash()(psoDesc));

        return pso;
    }
}