#pragma once

#include "src/Function/Renderer/Pass/RenderPass.h"
#include "src/Platform/DX12/DX12Common.h"
#include "src/Function/Renderer/DX12/DX12RHI.h"
#include "src/Function/Renderer/DX12/DX12Resource.h"
#include "src/Function/Renderer/DX12/DX12Asset.h"
#include "src/Function/Renderer/DX12/DX12Helper.h"
#include "src/Utility/MathTypes.h"

namespace Xunlan::DX12
{
    class GPass final : public RenderPass
    {
    public:

        virtual bool Initialize(const RenderPassInitDesc& initDesc) override;
        virtual void Shutdown() override;

        void CheckSize(Math::uint2 size);
        void PrepareForDepth();
        void PrepareForLight(Helper::Barrier::BarrierContainer& container);

        void RenderDepth(const DX12FrameInfo& frameInfo);
        void RenderLight(const DX12FrameInfo& frameInfo);

        uint32 GetMainBufferPosition() const { return m_mainGBuffer->GetSRV().index; }

    private:

        bool CreateBuffers(Math::uint2 size);
        void PrepareRenderFrame(DX12RHI* dx12RHI, const DX12FrameInfo& frameInfo);
        void FillPerObjectBuffer(DX12CBuffer& cBuffer, const DX12FrameInfo& frameInfo);
        void SetRootParams(GraphicsCommandList& cmdList, uint32 itemIndex);

    private:

        static constexpr Math::uint2 DEFAULT_DIMENSION = { 100, 100 };

        static constexpr D3D12_RESOURCE_STATES MAIN_GBUFFER_INIT_STATE = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        static constexpr D3D12_RESOURCE_STATES DEPTH_GBUFFER_INIT_STATE = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

    #if _DEBUG
        static constexpr float CLEAR_COLOR[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    #else
        static constexpr float CLEAR_COLOR[4] = {};
    #endif

        Math::uint2 m_dimension = DEFAULT_DIMENSION;

        std::unique_ptr<DX12RenderTexture> m_mainGBuffer;
        std::unique_ptr<DX12DepthTexture> m_depthGBuffer;

        struct GPassCache final
        {
            std::vector<Asset::DX12RenderItem> m_items;
            std::vector<D3D12_GPU_VIRTUAL_ADDRESS> m_perObjectBuffer;

            void Reset(std::vector<Asset::DX12RenderItem>&& items)
            {
                items.swap(m_items);
                m_perObjectBuffer.resize(m_items.size());
            }
            uint32 Size() const { return (uint32)m_items.size(); }

            MaterialType GetMaterialType(uint32 index) const { return Asset::Material::GetMaterial(m_items[index].materialID).type; }
            ID3D12RootSignature* GetRootSignature(uint32 index) const { return Asset::Material::GetMaterial(m_items[index].materialID).rootSignature.Get(); }
            ID3D12PipelineState* GetDepthPassPSO(uint32 index) const { return m_items[index].PSOs[0]; }
            ID3D12PipelineState* GetGPassPSO(uint32 index) const { return m_items[index].PSOs[1]; }

            D3D12_VERTEX_BUFFER_VIEW GetPositionView(uint32 index) const { return Asset::Submesh::GetSubmesh(m_items[index].submeshID).vertexBufferView; }
            D3D12_INDEX_BUFFER_VIEW GetIndexView(uint32 index) const { return Asset::Submesh::GetSubmesh(m_items[index].submeshID).indexBufferView; }
            D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology(uint32 index) const { return Asset::Submesh::GetSubmesh(m_items[index].submeshID).primitiveTopology; }
        };

        GPassCache m_cache;
    };
}