#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/Abstract/RHI.h"
#include "DX12Surface.h"
#include "DX12Command.h"
#include "DX12Resource.h"
#include "DX12GPUVisibleDescriptorHeap.h"
#include "DX12Upload.h"
#include "DX12Helper.h"
#include "DX12Material.h"
#include "Helper/d3dx12.h"

#include <array>
#include <unordered_map>
#include <mutex>

namespace Xunlan::DX12
{
    class DX12RHI final : public RHI
    {
    public:

        explicit DX12RHI(Platform platform);
        virtual ~DX12RHI() override;

        static DX12RHI& Instance() { return (DX12RHI&)(*ms_instance); }

        virtual void Flush() override;

        virtual uint32 GetWidth() override { return m_surface->GetWidth(); }
        virtual uint32 GetHeight() override { return m_surface->GetHeight(); }
        virtual void Resize(uint32 width, uint32 height) override;

        DXGI_FORMAT GetRTFormat() const { return m_currRTFormat; }
        DXGI_FORMAT GetDSFormat() const { return m_currDSFormat; }

        virtual Ref<RenderContext> CreateRenderContext() override;
        virtual void Execute(Ref<RenderContext>& context) override;
        virtual void Present() override { m_surface->Present(); }

        virtual void SetRenderTarget(const Ref<RenderContext>& context, const CRef<RenderTarget>& renderTarget) override;
        virtual void ClearRenderTarget(const Ref<RenderContext>& context, const CRef<RenderTarget>& renderTarget) override;
        virtual void ResetRenderTarget(const Ref<RenderContext>& context, const CRef<RenderTarget>& renderTarget) override;
        virtual void SetViewport(const Ref<RenderContext>& context, uint32 x, uint32 y, uint32 width, uint32 height) override;

        virtual Ref<Mesh> CreateMesh(const CRef<MeshRawData>& meshRawData) override;
        virtual Ref<Shader> CreateShader(ShaderType type, const std::filesystem::path& path, const std::string& functionName) override;
        virtual Ref<ImageTexture> CreateImageTexture(const CRef<RawTexture>& rawTexture) override;
        virtual Ref<RenderTarget> CreateRenderTarget(uint32 width, uint32 height, RenderTargetUsage usage) override;
        virtual Ref<RasterizerState> CreateRasterizerState(const RasterizerStateDesc& desc) override;
        virtual Ref<DepthStencilState> CreateDepthStencilState() override;
        virtual Ref<CBuffer> CreateConstantBuffer(CBufferType type, uint32 size) override;
        virtual Ref<Material> CreateMaterial(const std::string& name, MaterialType type, const ShaderList& shaderList) override;
        virtual Ref<RenderItem> CreateRenderItem(const Ref<Mesh>& mesh) override;
        virtual Ref<RenderItem> CreateRenderItem(const Ref<Mesh>& mesh, const std::vector<Ref<Material>>& materials) override;

    public:

        template<typename T>
        void DeferredRelease(Microsoft::WRL::ComPtr<T>& resource)
        {
            if (!resource) return;

            Microsoft::WRL::ComPtr<IUnknown> res;
            Check(resource.As(&res));
            DeferredRelease(res);
            resource.Reset();
        }
        void DeferredRelease(Microsoft::WRL::ComPtr<IUnknown>& resource);
        void SetDeferredReleaseFlag(uint32 frameIndex) { m_deferredReleaseFlag[frameIndex] = 1; }

        uint32 GetCurrFrameIndex() const { return m_mainCommand->GetCurrFrameIndex(); }

        Device& GetDevice() { return *m_device.Get(); }
        DX12Surface& GetSurface() { return *m_surface; }
        DX12DescriptorHeap& GetRTVHeap() { return *m_rtvHeap; }
        DX12DescriptorHeap& GetDSVHeap() { return *m_dsvHeap; }
        DX12DescriptorHeap& GetSRVHeap() { return *m_srvHeap; }
        UploadContext& GetUploadContext() { return *m_uploadContext; }
        ID3D12RootSignature* GetDefaultRootSig() const { return m_defaultRootSig.Get(); }
        auto& GetPSOContainer() { return m_psoContainer; }

        void EnableDebugLayer();
        void CreateDevice();
        void CreateDescriptorHeap();
        void CreateRootSignature();

        std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers() const;
        //D3D12_INPUT_LAYOUT_DESC GetInputLayout() const;

        void ProcessDeferredRelease(uint32 frameIndex);
        void ReleaseHeapsAndDeferredResources();

    private:

        Microsoft::WRL::ComPtr<Factory> m_factory;
        Microsoft::WRL::ComPtr<Device> m_device;

        std::unique_ptr<DX12Command> m_mainCommand;
        std::unique_ptr<DX12Surface> m_surface;
        std::unique_ptr<DX12DescriptorHeap> m_rtvHeap;
        std::unique_ptr<DX12DescriptorHeap> m_dsvHeap;
        std::unique_ptr<DX12DescriptorHeap> m_srvHeap;
        std::unique_ptr<UploadContext> m_uploadContext;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_defaultRootSig;
        std::unordered_map<DX12PSO, Microsoft::WRL::ComPtr<ID3D12PipelineState>, DX12PSO::Hash> m_psoContainer;

        DXGI_FORMAT m_currRTFormat;
        DXGI_FORMAT m_currDSFormat;

        std::vector<Microsoft::WRL::ComPtr<IUnknown>> m_deferredReleaseResources[NUM_FRAME_BUFFERS];
        uint32 m_deferredReleaseFlag[NUM_FRAME_BUFFERS] = {};
        std::mutex m_deferredReleaseMutex;

        static constexpr D3D12_INPUT_ELEMENT_DESC m_elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
    };
}