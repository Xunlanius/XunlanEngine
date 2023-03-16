#include "DX12GPass.h"
#include "Renderer/DX12/DX12Core.h"
#include "Renderer/DX12/DX12Shader.h"

namespace Xunlan::Graphics::DX12::GPass
{
    using namespace Microsoft::WRL;

    namespace
    {
        constexpr Math::U32Vector2 DEFAULT_DIMENSION = { 100, 100 };

        constexpr DXGI_FORMAT MAIN_GBUFFER_FORMAT = DXGI_FORMAT_R16G16B16A16_FLOAT;
        constexpr DXGI_FORMAT DEPTH_GBUFFER_FORMAT = DXGI_FORMAT_D32_FLOAT;

        constexpr D3D12_RESOURCE_STATES MAIN_GBUFFER_INIT_STATE = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        constexpr D3D12_RESOURCE_STATES DEPTH_GBUFFER_INIT_STATE = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

#if _DEBUG
        constexpr float CLEAR_COLOR[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
#else
        constexpr float CLEAR_COLOR[4] = {};
#endif

        Math::U32Vector2 g_dimension = DEFAULT_DIMENSION;

        std::unique_ptr<RenderTexture> g_mainGBuffer = nullptr;
        std::unique_ptr<DepthTexture> g_depthGBuffer = nullptr;

        ComPtr<ID3D12RootSignature> g_rootSignature = nullptr;
        ComPtr<ID3D12PipelineState> g_PSO = nullptr;

        enum class ParamIndex : uint32
        {
            Constants,

            Count,
        };

        bool CreateBuffers(Math::U32Vector2 size)
        {
            assert(size.x > 0 && size.y > 0);

            if (g_mainGBuffer) g_mainGBuffer->Release();
            if (g_depthGBuffer) g_depthGBuffer->Release();

            D3D12_RESOURCE_DESC resourceDesc = {};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            resourceDesc.Alignment = 0;     // 64KB (or 4MB for MSAA)
            resourceDesc.Width = size.x;
            resourceDesc.Height = size.y;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 0;     // Make place for all mip levels
            resourceDesc.Format = MAIN_GBUFFER_FORMAT;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

            // Create main GBuffer
            {
                TextureInitDesc desc = {};
                desc.mode = TextureInitDesc::Mode::CreateDefault;
                desc.resourceDesc = &resourceDesc;
                desc.initState = MAIN_GBUFFER_INIT_STATE;
                desc.clearValue.Format = resourceDesc.Format;
                memcpy(desc.clearValue.Color, CLEAR_COLOR, sizeof(CLEAR_COLOR));

                g_mainGBuffer = std::make_unique<RenderTexture>(desc);
                if (!g_mainGBuffer->GetResource()) { assert(false); return false; }
                NAME_OBJECT(g_mainGBuffer->GetResource(), L"GPass Main Buffer");
            }

            resourceDesc.MipLevels = 1;
            resourceDesc.Format = DEPTH_GBUFFER_FORMAT;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

            // Create depth buffer
            {
                TextureInitDesc desc = {};
                desc.mode = TextureInitDesc::Mode::CreateDefault;
                desc.resourceDesc = &resourceDesc;
                desc.initState = DEPTH_GBUFFER_INIT_STATE;
                desc.clearValue.Format = resourceDesc.Format;
                desc.clearValue.DepthStencil.Depth = 0.0f;
                desc.clearValue.DepthStencil.Stencil = 0;

                g_depthGBuffer = std::make_unique<DepthTexture>(desc);
                if (!g_depthGBuffer->GetResource()) { assert(false); return false; }
                NAME_OBJECT(g_depthGBuffer->GetResource(), L"GPass Depth Buffer");
            }

            return true;
        }
        bool CreateRootSignatureAndPSO()
        {
            assert(!g_rootSignature && !g_PSO);

            Device& device = Core::GetDevice();

            // Create GPass root signature
            {
                Helper::RootSig::RootParameter params[(uint32)ParamIndex::Count] = {};
                params[(uint32)ParamIndex::Constants].InitAsConstants(3, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);

                const Helper::RootSig::RootSignatureDesc desc(_countof(params), params);

                g_rootSignature = Helper::RootSig::CreateRootSignature(device, desc);
                if (!g_rootSignature) return false;
                NAME_OBJECT(g_rootSignature, L"GPass Root Signature");
            }

            // Create GPass PSO
            {
                struct
                {
                    Helper::PSO::Subobject_RootSignature         rootSignature = g_rootSignature.Get();
                    Helper::PSO::Subobject_VS                    vs = Shader::GetEngineShaderByteCode(Shader::EngineShader::TriangleVS);
                    Helper::PSO::Subobject_PS                    ps = Shader::GetEngineShaderByteCode(Shader::EngineShader::TrianglePS);
                    Helper::PSO::Subobject_Rasterizer            rasterizer = Helper::Rasterizer::NO_CULL;
                    Helper::PSO::Subobject_PrimitiveTopology     primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                    Helper::PSO::Subobject_RenderTargetFormats   renderTargetFormats;
                    Helper::PSO::Subobject_DepthStencilFormat    depthStencilFormat = DEPTH_GBUFFER_FORMAT;
                    Helper::PSO::Subobject_DepthStencil1         depthStencil = Helper::DepthStencil::DISABLED;
                } stream;

                D3D12_RT_FORMAT_ARRAY rtFormatArray = {};
                rtFormatArray.NumRenderTargets = 1;
                rtFormatArray.RTFormats[0] = MAIN_GBUFFER_FORMAT;

                stream.renderTargetFormats = rtFormatArray;

                g_PSO = Helper::PSO::CreatePipelineState(device, &stream, sizeof(stream));
                if (!g_PSO) return false;
                NAME_OBJECT(g_PSO, L"GPass PSO");
            }

            return true;
        }
    }

    bool Initialize() { return CreateBuffers(DEFAULT_DIMENSION) && CreateRootSignatureAndPSO(); }
    void Shutdown()
    {
        g_mainGBuffer->Release();
        g_depthGBuffer->Release();

        g_dimension = DEFAULT_DIMENSION;

        g_PSO.Reset();
        g_rootSignature.Reset();
    }

    const RenderTexture& GetMainBuffer() { return *g_mainGBuffer; }
    const DepthTexture& GetDepthBuffer() { return *g_depthGBuffer; }

    void CheckSize(Math::U32Vector2 size)
    {
        if (size.x <= g_dimension.x || size.y <= g_dimension.y) return;

        g_dimension = { std::max(size.x, g_dimension.x), std::max(size.y, g_dimension.y) };
        CreateBuffers(g_dimension);
    }

    void AddTransitionForDepthPrepass(Helper::Barrier::BarrierContainer& container)
    {
        container.AddTransition(*g_depthGBuffer->GetResource(), DEPTH_GBUFFER_INIT_STATE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }
    void AddTransitionForGPass(Helper::Barrier::BarrierContainer& container)
    {
        container.AddTransition(*g_mainGBuffer->GetResource(), MAIN_GBUFFER_INIT_STATE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        container.AddTransition(*g_depthGBuffer->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, DEPTH_GBUFFER_INIT_STATE);
    }
    void AddTransitionForPostProcess(Helper::Barrier::BarrierContainer& container)
    {
        container.AddTransition(*g_mainGBuffer->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, MAIN_GBUFFER_INIT_STATE);
    }

    void SetRenderTargetForDepthPrepass(GraphicsCommandList* cmdList)
    {
        const D3D12_CPU_DESCRIPTOR_HANDLE dsv = g_depthGBuffer->GetDSV();

        cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0.0f, 0, 0, nullptr);
        cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsv);
    }
    void SetRenderTargetForGPass(GraphicsCommandList* cmdList)
    {
        const D3D12_CPU_DESCRIPTOR_HANDLE rtv = g_mainGBuffer->GetRTV(0);
        const D3D12_CPU_DESCRIPTOR_HANDLE dsv = g_depthGBuffer->GetDSV();

        cmdList->ClearRenderTargetView(rtv, CLEAR_COLOR, 0, nullptr);
        cmdList->OMSetRenderTargets(1, &rtv, TRUE, &dsv);
    }

    void RenderDepthPrepass(GraphicsCommandList* cmdList, const FrameInfo& frameInfo)
    {
    }
    void Render(GraphicsCommandList* cmdList, const FrameInfo& frameInfo)
    {
        cmdList->SetGraphicsRootSignature(g_rootSignature.Get());
        cmdList->SetPipelineState(g_PSO.Get());

        static uint32 frame = 0;
        struct
        {
            float width;
            float height;
            uint32 frame;
        } constants = { (float)frameInfo.surfaceWidth, (float)frameInfo.surfaceHeight, ++frame };

        cmdList->SetGraphicsRoot32BitConstants((uint32)ParamIndex::Constants, 3, &constants, 0);

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->DrawInstanced(3, 1, 0, 0);
    }
}