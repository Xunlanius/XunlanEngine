#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/Material.h"
#include "DX12Shader.h"
#include "DX12RasterizerState.h"
#include "DX12DepthStencilState.h"

namespace Xunlan::DX12
{
    struct DX12PSO final
    {
        CRef<DX12Shader> m_VS;
        CRef<DX12Shader> m_PS;
        CRef<DX12RasterizerState> m_rasterizerState;
        CRef<DX12DepthStencilState> m_depthStencilState;
        uint32 numRTs;
        DXGI_FORMAT m_rtFormat;
        DXGI_FORMAT m_dsFormat;

        friend bool operator==(const DX12PSO& lhs, const DX12PSO& rhs);

        struct Hash
        {
            size_t operator()(const DX12PSO& key) const;
        };
    };

    class DX12Material final : public Material
    {
    public:

        explicit DX12Material(const std::string& name, MaterialType type, const ShaderList& shaderList)
            : Material(name, type, shaderList) {}

    public:

        virtual void Apply(const Ref<RenderContext>& context) const override;

    private:

        void CollectTextureSRVs(const Ref<RenderContext>& context) const;
        void BindTextureSRVs(const Ref<RenderContext>& context, const std::vector<uint32>& textureIndices) const;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPSO() const;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> CreatePSO(const DX12PSO& psoDesc) const;
    };
}