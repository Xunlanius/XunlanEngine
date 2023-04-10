#include "GPass.h"
#include "src/Function/Renderer/DX12/DX12RootParameter.h"
#include "src/Function/World/Component/Transformer.h"
#include "src/Function/World/Component/Camera.h"

namespace Xunlan::DX12
{
    bool GPass::Initialize(const RenderPassInitDesc& initDesc)
    {
        return CreateBuffers(DEFAULT_DIMENSION);
    }
    void GPass::Shutdown()
    {
        m_mainGBuffer->Release();
        m_depthGBuffer->Release();
        m_dimension = DEFAULT_DIMENSION;
    }

    void GPass::CheckSize(Math::uint2 size)
    {
        if (size.x == m_dimension.x || size.y == m_dimension.y) return;

        m_dimension = { size.x, size.y };
        CreateBuffers(m_dimension);
    }
    void GPass::PrepareForDepth()
    {
        DX12RHI* dx12RHI = (DX12RHI*)(&RHI::Instance());
        GraphicsCommandList& cmdList = dx12RHI->GetCmdList();

        Helper::Barrier::Transition(cmdList, *m_depthGBuffer->GetResource(), DEPTH_GBUFFER_INIT_STATE, D3D12_RESOURCE_STATE_DEPTH_WRITE);

        const D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_depthGBuffer->GetDSV();

        cmdList.ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        cmdList.OMSetRenderTargets(0, nullptr, FALSE, &dsv);
    }
    void GPass::PrepareForLight(Helper::Barrier::BarrierContainer& container)
    {
        DX12RHI* dx12RHI = (DX12RHI*)(&RHI::Instance());
        GraphicsCommandList& cmdList = dx12RHI->GetCmdList();

        container.AddTransition(*m_mainGBuffer->GetResource(), MAIN_GBUFFER_INIT_STATE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        container.AddTransition(*m_depthGBuffer->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, DEPTH_GBUFFER_INIT_STATE);
        container.Commit(cmdList);

        const D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_mainGBuffer->GetRTV(0);
        const D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_depthGBuffer->GetDSV();

        cmdList.ClearRenderTargetView(rtv, CLEAR_COLOR, 0, nullptr);
        cmdList.OMSetRenderTargets(1, &rtv, TRUE, &dsv);
    }

    void GPass::RenderDepth(const DX12FrameInfo& frameInfo)
    {
        DX12RHI* dx12RHI = (DX12RHI*)(&RHI::Instance());
        GraphicsCommandList& cmdList = dx12RHI->GetCmdList();

        PrepareRenderFrame(dx12RHI, frameInfo);

        const uint32 numRenderItems = m_cache.Size();
        ID3D12RootSignature* currRootSignature = nullptr;
        ID3D12PipelineState* currPipelineState = nullptr;

        for (uint32 i = 0; i < numRenderItems; ++i)
        {
            ID3D12RootSignature* rootSignature = m_cache.GetRootSignature(i);
            ID3D12PipelineState* depthPassPSO = m_cache.GetDepthPassPSO(i);

            if (currRootSignature != rootSignature)
            {
                currRootSignature = rootSignature;
                cmdList.SetGraphicsRootSignature(currRootSignature);
                cmdList.SetGraphicsRootConstantBufferView((uint32)OpaqueRootParam::PER_FRAME, frameInfo.perFrameDataAddress);
            }

            if (currPipelineState != depthPassPSO)
            {
                currPipelineState = depthPassPSO;
                cmdList.SetPipelineState(currPipelineState);
            }

            SetRootParams(cmdList, i);

            const D3D12_INDEX_BUFFER_VIEW indexBufferView = m_cache.GetIndexView(i);
            const uint32 numIndices = indexBufferView.SizeInBytes >> 2;
            cmdList.IASetIndexBuffer(&indexBufferView);
            cmdList.IASetPrimitiveTopology(m_cache.GetPrimitiveTopology(i));
            cmdList.DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
        }
    }
    void GPass::RenderLight(const DX12FrameInfo& frameInfo)
    {
        DX12RHI* dx12RHI = (DX12RHI*)(&RHI::Instance());
        GraphicsCommandList& cmdList = dx12RHI->GetCmdList();

        PrepareRenderFrame(dx12RHI, frameInfo);

        const uint32 numRenderItems = m_cache.Size();
        ID3D12RootSignature* currRootSignature = nullptr;
        ID3D12PipelineState* currPipelineState = nullptr;

        for (uint32 i = 0; i < numRenderItems; ++i)
        {
            ID3D12RootSignature* rootSignature = m_cache.GetRootSignature(i);
            ID3D12PipelineState* gPassPSO = m_cache.GetGPassPSO(i);

            if (currRootSignature != rootSignature)
            {
                currRootSignature = rootSignature;
                cmdList.SetGraphicsRootSignature(currRootSignature);
                cmdList.SetGraphicsRootConstantBufferView((uint32)OpaqueRootParam::PER_FRAME, frameInfo.perFrameDataAddress);
            }

            if (currPipelineState != gPassPSO)
            {
                currPipelineState = gPassPSO;
                cmdList.SetPipelineState(currPipelineState);
            }

            SetRootParams(cmdList, i);

            const D3D12_INDEX_BUFFER_VIEW indexBufferView = m_cache.GetIndexView(i);
            const uint32 numIndices = indexBufferView.SizeInBytes >> 2;
            cmdList.IASetIndexBuffer(&indexBufferView);
            cmdList.IASetPrimitiveTopology(m_cache.GetPrimitiveTopology(i));
            cmdList.DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
        }

        Helper::Barrier::Transition(cmdList, *m_mainGBuffer->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, MAIN_GBUFFER_INIT_STATE);
    }

    bool GPass::CreateBuffers(Math::uint2 size)
    {
        assert(size.x > 0 && size.y > 0);

        if (m_mainGBuffer) m_mainGBuffer->Release();
        if (m_depthGBuffer) m_depthGBuffer->Release();

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

            m_mainGBuffer = std::make_unique<DX12RenderTexture>(desc);
            if (!m_mainGBuffer->GetResource()) { assert(false); return false; }
            NAME_OBJECT(m_mainGBuffer->GetResource(), L"GPass Main Buffer");
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
            desc.clearValue.DepthStencil.Depth = 1.0f;
            desc.clearValue.DepthStencil.Stencil = 0;

            m_depthGBuffer = std::make_unique<DX12DepthTexture>(desc);
            if (!m_depthGBuffer->GetResource()) { assert(false); return false; }
            NAME_OBJECT(m_depthGBuffer->GetResource(), L"GPass Depth Buffer");
        }

        return true;
    }
    void GPass::PrepareRenderFrame(DX12RHI* dx12RHI, const DX12FrameInfo& frameInfo)
    {
        assert(frameInfo.renderResource && frameInfo.renderResource->m_renderItemIDs.size() > 0);

        m_cache.Reset(Asset::RenderItem::GetRenderItems(*frameInfo.renderResource));
        FillPerObjectBuffer(dx12RHI->GetCBuffer(), frameInfo);
    }
    void GPass::FillPerObjectBuffer(DX12CBuffer& cBuffer, const DX12FrameInfo& frameInfo)
    {
        using namespace DirectX;

        const HLSL::PerFrameBuffer& perFrameBuffer = frameInfo.renderResource->m_perFrameBuffer;
        const XMMATRIX view = XMLoadFloat4x4(&perFrameBuffer.view);
        const XMMATRIX proj = XMLoadFloat4x4(&perFrameBuffer.proj);

        const uint32 numItems = m_cache.Size();
        for (uint32 i = 0; i < numItems; ++i)
        {
            auto [transformer] = Singleton<ECS::ECSManager>::Instance().GetComponent<TransformerComponent>(m_cache.m_items[i].entity);

            const XMMATRIX world = XMLoadFloat4x4(&GetWorld(transformer));
            const XMMATRIX invWorld = XMMatrixInverse(nullptr, world);
            const XMMATRIX worldViewProj = world * view * proj;

            HLSL::PerObjectBuffer buffer = {};
            XMStoreFloat4x4(&buffer.world, world);
            XMStoreFloat4x4(&buffer.invWorld, invWorld);
            XMStoreFloat4x4(&buffer.worldViewProj, worldViewProj);

            m_cache.m_perObjectBuffer[i] = cBuffer.Allocate(buffer);
        }
    }
    void GPass::SetRootParams(GraphicsCommandList& cmdList, uint32 itemIndex)
    {
        assert(itemIndex < m_cache.Size());

        const MaterialType type = m_cache.GetMaterialType(itemIndex);
        switch (type)
        {
            case MaterialType::Opaque:
            {
                cmdList.SetGraphicsRootConstantBufferView((uint32)OpaqueRootParam::PER_OBJECT, m_cache.m_perObjectBuffer[itemIndex]);
                cmdList.SetGraphicsRootShaderResourceView((uint32)OpaqueRootParam::VertexBuffer, m_cache.GetPositionView(itemIndex).BufferLocation);
            }
        }
    }
}