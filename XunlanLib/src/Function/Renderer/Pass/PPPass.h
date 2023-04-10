#pragma once

#include "src/Function/Renderer/Pass/RenderPass.h"
#include "src/Platform/DX12/DX12Common.h"
#include "src/Tool/Shader/ShaderCompileSystem.h"

namespace Xunlan::DX12
{
    class PostProcessPass final : public RenderPass
    {
    public:

        virtual bool Initialize(const RenderPassInitDesc& initDesc) override;
        virtual void Shutdown() override;
        void Prepare(D3D12_CPU_DESCRIPTOR_HANDLE rtv, uint32 index);
        void Render();

    private:

        static D3D12_SHADER_BYTECODE GetEngineShader(EngineShader id);

    private:

        enum class Param : uint32
        {
            Constants,
            Count,
        };

        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PSO;

        D3D12_CPU_DESCRIPTOR_HANDLE m_rtv;
        uint32 m_index;
    };
}