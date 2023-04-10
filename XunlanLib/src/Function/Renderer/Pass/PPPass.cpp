#include "PostProcessPass.h"
#include "src/Function/Renderer/DX12/DX12RHI.h"
#include "src/Function/Renderer/DX12/DX12Surface.h"
#include "src/Function/Renderer/DX12/DX12Helper.h"
#include "src/Function/Core/RuntimeContext.h"

namespace Xunlan::DX12
{
    bool PostProcessPass::Initialize(const RenderPassInitDesc& initDesc)
    {
        DX12RHI* dx12RHI = (DX12RHI*)(&RHI::Instance());

        Device& device = dx12RHI->GetDevice();

        // Create Post-Process root signature
        {
            Helper::RootSig::RootParameter params[(uint32)Param::Count] = {};
            params[(uint32)Param::Constants].InitAsConstants(1, 1, D3D12_SHADER_VISIBILITY_PIXEL);

            Helper::RootSig::RootSignatureDesc desc(_countof(params), params);
            desc.Flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

            m_rootSignature = Helper::RootSig::CreateRootSignature(device, desc);
            if (!m_rootSignature) return false;
            NAME_OBJECT(m_rootSignature, L"PostProcess Root Signature");
        }

        // Create Post-Process PSO
        {
            struct
            {
                Helper::PSO::Subobject_RootSignature         rootSignature;
                Helper::PSO::Subobject_VS                    vs;
                Helper::PSO::Subobject_PS                    ps;
                Helper::PSO::Subobject_Rasterizer            rasterizer;
                Helper::PSO::Subobject_PrimitiveTopology     primitiveTopology;
                Helper::PSO::Subobject_RenderTargetFormats   renderTargetFormats;
            } stream;

            stream.rootSignature = m_rootSignature.Get();
            stream.vs = GetEngineShader(EngineShader::TriangleVS);
            stream.ps = GetEngineShader(EngineShader::PostProcessPS);
            stream.rasterizer = Helper::Rasterizer::NO_CULL;
            stream.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            D3D12_RT_FORMAT_ARRAY rtFormatArray = {};
            rtFormatArray.NumRenderTargets = 1;
            rtFormatArray.RTFormats[0] = DX12Surface::DEFAULT_RENDER_TARGET_FORMAT;

            stream.renderTargetFormats = rtFormatArray;

            m_PSO = Helper::PSO::CreatePipelineState(device, &stream, sizeof(stream));
            if (!m_PSO) return false;
            NAME_OBJECT(m_PSO, L"PostProcess PSO");
        }

        return true;
    }

    void PostProcessPass::Shutdown()
    {
        m_PSO.Reset();
        m_rootSignature.Reset();
    }

    void PostProcessPass::Prepare(D3D12_CPU_DESCRIPTOR_HANDLE rtv, uint32 index)
    {
        m_rtv = rtv;
        m_index = index;
    }

    void PostProcessPass::Render()
    {
        DX12RHI* dx12RHI = (DX12RHI*)(&RHI::Instance());
        GraphicsCommandList& cmdList = dx12RHI->GetCmdList();

        cmdList.OMSetRenderTargets(1, &m_rtv, TRUE, nullptr);

        cmdList.SetGraphicsRootSignature(m_rootSignature.Get());
        cmdList.SetPipelineState(m_PSO.Get());

        cmdList.SetGraphicsRoot32BitConstant((uint32)Param::Constants, m_index, 0);

        cmdList.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList.DrawInstanced(3, 1, 0, 0);
    }

    D3D12_SHADER_BYTECODE PostProcessPass::GetEngineShader(EngineShader id)
    {
        const CompiledShader& shader = g_runtimeContext.m_shaderCompileSystem->GetEngineShader(id);

        D3D12_SHADER_BYTECODE byteCode = {};
        byteCode.BytecodeLength = shader.byteCode.size();
        byteCode.pShaderBytecode = shader.byteCode.data();
        return byteCode;
    }
}