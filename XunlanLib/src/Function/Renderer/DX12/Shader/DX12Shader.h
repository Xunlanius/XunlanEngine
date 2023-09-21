#pragma once

#include "src/Function/Renderer/Abstract/Shader.h"
#include "../DX12Common.h"
#include "../DX12RasterizerState.h"
#include "../DX12DepthStencilState.h"
#include "../DX12CBuffer.h"
#include "../DX12DataBuffer.h"
#include "../DX12RenderContext.h"
#include "DX12ShaderCompiler.h"
#include "../Helper/d3dx12.h"

#include <unordered_map>
#include <filesystem>

namespace Xunlan::DX12
{
    enum class DX12ShaderType : uint32
    {
        VS,
        PS,
    };

    struct ShaderParam
    {
        DX12ShaderType m_shaderType;
        std::string m_name;
        uint32 m_bindPoint;
        uint32 m_bindCount;
        uint32 m_registerSpace;
    };

    struct CBVParam final : public ShaderParam
    {
        D3D12_GPU_VIRTUAL_ADDRESS m_gpuAddress;
    };

    struct SRVParam final : public ShaderParam
    {
        D3D12_GPU_VIRTUAL_ADDRESS m_gpuAddress;
    };

    struct UAVParam final : public ShaderParam
    {
        D3D12_GPU_VIRTUAL_ADDRESS m_gpuAddress;
    };

    struct SamplerParam final : public ShaderParam {};

    struct DX12PSO final
    {
        D3D12_SHADER_BYTECODE m_vs;
        D3D12_SHADER_BYTECODE m_ps;

        CRef<DX12RasterizerState> m_rasterizerState;
        CRef<DX12DepthStencilState> m_depthStencilState;

        std::vector<DXGI_FORMAT> m_rtFormats;
        DXGI_FORMAT m_dsFormat;

        friend bool operator==(const DX12PSO& lhs, const DX12PSO& rhs);

        struct Hash
        {
            size_t operator()(const DX12PSO& key) const;
        };
    };

    class DX12Shader final : public Shader
    {
    public:

        DX12Shader(const std::string& name, const ShaderInitDesc& desc, const std::filesystem::path& path);

    public:

        virtual bool HasParam(const std::string& name) const override;
        virtual bool SetParam(const std::string& name, CRef<CBuffer> cBuffer) override;
        virtual bool SetParam(const std::string& name, CRef<DataBuffer> buffer) override;

        void Apply(Ref<RenderContext> context);

        void Bind(Ref<RenderContext> context);

        D3D12_SHADER_BYTECODE GetByteCode(DX12ShaderType type) const;
        ID3D12RootSignature* GetRootSig() const { return m_rootSig.Get(); }

    private:

        void StoreByteCode(DX12ShaderType type, Microsoft::WRL::ComPtr<IDxcBlob> compiledShader);
        void StoreParam(DX12ShaderType type, Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflection);

        void CreateRootSig();
        static std::vector<CD3DX12_STATIC_SAMPLER_DESC> GetStaticSamplers();

        void CheckBinding() const;
        void ClearBinding();

        auto FindParam(auto&& params, const std::string& name) const;
        auto FindParam(auto&& params, const std::string& name, uint32 bindPoint) const;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> TryGetPSO() const;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> CreatePSO(const DX12PSO& psoDesc) const;
        DX12PSO BuildPSODesc() const;

    private:

        std::unordered_map<DX12ShaderType, std::vector<byte>> m_shaderMap;

        std::vector<CBVParam> m_paramCBVs;
        std::vector<SRVParam> m_paramSRVs;
        std::vector<UAVParam> m_paramUAVs;
        std::vector<SamplerParam> m_paramSamplers;

        int m_indexBaseCBV = -1;
        int m_indexBaseSRV = -1;
        int m_indexBaseUAV = -1;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSig;
    };

    inline auto DX12Shader::FindParam(auto&& params, const std::string& name) const
    {
        return std::find_if(
            params.begin(),
            params.end(),
            [&name](const auto& param) { return param.m_name == name; }
        );
    }

    inline auto DX12Shader::FindParam(auto&& params, const std::string& name, uint32 bindPoint) const
    {
        return std::find_if(
            params.begin(),
            params.end(),
            [&name, bindPoint](const auto& param)
            {
                return param.m_name == name && param.m_bindPoint == bindPoint;
            }
        );
    }
}