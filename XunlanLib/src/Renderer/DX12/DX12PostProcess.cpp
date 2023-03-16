#include "DX12PostProcess.h"
#include "Renderer/DX12/DX12Core.h"
#include "Renderer/DX12/DX12Surface.h"
#include "Renderer/DX12/DX12Shader.h"
#include "Renderer/DX12/DX12GPass.h"

namespace Xunlan::Graphics::DX12::PostProcess
{
    using namespace Microsoft::WRL;

    namespace
    {
        enum class ParamIndex : uint32
        {
            Constants,
            DescriptorTable,

            Count,
        };

        ComPtr<ID3D12RootSignature> g_rootSignature = nullptr;
        ComPtr<ID3D12PipelineState> g_PSO = nullptr;

        bool CreateRootSignatureAndPSO()
        {
            assert(!g_rootSignature && !g_PSO);

            Device& device = Core::GetDevice();

            // Create Post-Process root signature
            {
                Helper::RootSig::DescriptorRange range(
                    D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                    D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
                    0,
                    0,
                    D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE
                );

                Helper::RootSig::RootParameter params[(uint32)ParamIndex::Count] = {};
                params[(uint32)ParamIndex::Constants].InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
                params[(uint32)ParamIndex::DescriptorTable].InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_PIXEL);

                const Helper::RootSig::RootSignatureDesc desc(_countof(params), params);

                g_rootSignature = Helper::RootSig::CreateRootSignature(device, desc);
                if (!g_rootSignature) return false;
                NAME_OBJECT(g_rootSignature, L"PostProcess Root Signature");
            }

            // Create Post-Process PSO
            {
                struct
                {
                    Helper::PSO::Subobject_RootSignature         rootSignature = g_rootSignature.Get();
                    Helper::PSO::Subobject_VS                    vs = Shader::GetEngineShaderByteCode(Shader::EngineShader::TriangleVS);
                    Helper::PSO::Subobject_PS                    ps = Shader::GetEngineShaderByteCode(Shader::EngineShader::PostProcessPS);
                    Helper::PSO::Subobject_Rasterizer            rasterizer = Helper::Rasterizer::NO_CULL;
                    Helper::PSO::Subobject_PrimitiveTopology     primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                    Helper::PSO::Subobject_RenderTargetFormats   renderTargetFormats;
                } stream;

                D3D12_RT_FORMAT_ARRAY rtFormatArray = {};
                rtFormatArray.NumRenderTargets = 1;
                rtFormatArray.RTFormats[0] = DX12Surface::DEFAULT_RENDER_TARGET_FORMAT;

                stream.renderTargetFormats = rtFormatArray;

                g_PSO = Helper::PSO::CreatePipelineState(device, &stream, sizeof(stream));
                if (!g_PSO) return false;
                NAME_OBJECT(g_PSO, L"PostProcess PSO");
            }

            return true;
        }
    }

    bool Initialize() { return CreateRootSignatureAndPSO(); }
    void Shutdown()
    {
        g_PSO.Reset();
        g_rootSignature.Reset();
    }

    void PostProcess(GraphicsCommandList& cmdList, D3D12_CPU_DESCRIPTOR_HANDLE rtv)
    {
        cmdList.OMSetRenderTargets(1, &rtv, TRUE, nullptr);

        cmdList.SetGraphicsRootSignature(g_rootSignature.Get());
        cmdList.SetPipelineState(g_PSO.Get());

        cmdList.SetGraphicsRoot32BitConstant((uint32)ParamIndex::Constants, GPass::GetMainBuffer().GetSRV().index, 0);
        cmdList.SetGraphicsRootDescriptorTable((uint32)ParamIndex::DescriptorTable, Core::GetSRVHeap().GetAddressGPU());

        cmdList.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList.DrawInstanced(3, 1, 0, 0);
    }
}