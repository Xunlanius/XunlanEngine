#include "DX12Core.h"
#include "Utility/Pool.h"
#include "Renderer/DX12/DX12Command.h"
#include "Renderer/DX12/DX12Upload.h"
#include "Renderer/DX12/DX12Surface.h"
#include "Renderer/DX12/DX12Shader.h"
#include "Renderer/DX12/DX12GPass.h"
#include "Renderer/DX12/DX12PostProcess.h"

namespace Xunlan::Graphics::DX12::Core
{
    using namespace Microsoft::WRL;

    namespace
    {
        ComPtr<Factory> g_factory = nullptr;
        ComPtr<Device> g_device = nullptr;
        std::unique_ptr<DX12Command> g_command = nullptr;
        Helper::Barrier::BarrierContainer g_barrierContainer = {};

        std::unique_ptr<DescriptorHeap> g_rtvHeap = nullptr;
        std::unique_ptr<DescriptorHeap> g_dsvHeap = nullptr;
        std::unique_ptr<DescriptorHeap> g_srvHeap = nullptr;
        std::unique_ptr<DescriptorHeap> g_uavHeap = nullptr;

        std::vector<ComPtr<IUnknown>> g_deferredReleaseResources[NUM_FRAME_BUFFERS];
        uint32 g_deferredReleaseFlag[NUM_FRAME_BUFFERS] = {};
        std::mutex g_deferredReleaseMutex = {};

        Utility::ObjectPool<DX12Surface> g_surfaces;

        void EnableDebugLayer()
        {
        #if defined (_DEBUG)
            ComPtr<ID3D12Debug3> debugInterface;
            Check(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));

            debugInterface->EnableDebugLayer();
        #if 0
            debugInterface->SetEnableGPUBasedValidation(TRUE);
        #endif
        #endif
        }
        void CreateDevice()
        {
            //ID3D12Deviceָ������Ⱦ�Ķ���������Ҫָ��Ӳ��������device
            //IDXGIAdapterָ��ʾ����������ͨ��IDXGIFactory��EnumAdapters��������ȡ

            uint32 factoryFlags = 0;
        #if defined _DEBUG
            factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        #endif

            Check(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&g_factory)));

            ComPtr<IDXGIAdapter4> targetAdapter;
            ComPtr<IDXGIAdapter4> currAdapter;
            uint64 maxDedicatedVideoMemory = 0;

            //������ֵ����ϵͳ��adapter�ĸ�����EnumAdapters�᷵��DXGI_ERROR_NOT_FOUND
            for (uint32 i = 0;
                g_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&currAdapter)) != DXGI_ERROR_NOT_FOUND;
                ++i)
            {
                DXGI_ADAPTER_DESC1 desc = {};
                currAdapter->GetDesc1(&desc);

                //��adapter�Ķ�ռ�Դ���Ϊ����adapter�û��ı�׼
                if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && maxDedicatedVideoMemory < desc.DedicatedVideoMemory)
                {
                    maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
                    targetAdapter = currAdapter;
                }
            }

            //pAdapter����ʾ��������GPU����ָ�룬��Ϊ��ָ�룬��ΪĬ�ϵ�����ʾ��������һ����Ĭ��ʹ�õĶ����Կ���
            //MinimumFeatureLevel��Ҫ�����ʾ���������֧�ֵ�DirectX�İ汾�����޷�֧�ָð汾,�ú����᷵�ش���ʧ�ܵ��쳣����
            Check(D3D12CreateDevice(targetAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&g_device)));
            NAME_OBJECT(g_device, L"Main Device");
        }
        bool CreateDescriptorHeap()
        {
            g_rtvHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            g_dsvHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            g_srvHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            g_uavHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            if (!g_rtvHeap->Initialize(512, false)) return false;
            NAME_OBJECT(g_rtvHeap->GetHeap(), L"RTV Descriptor Heap");

            if (!g_dsvHeap->Initialize(512, false)) return false;
            NAME_OBJECT(g_dsvHeap->GetHeap(), L"DSV Descriptor Heap");

            if (!g_srvHeap->Initialize(4096, true)) return false;
            NAME_OBJECT(g_srvHeap->GetHeap(), L"SRV Descriptor Heap");

            if (!g_uavHeap->Initialize(512, false)) return false;
            NAME_OBJECT(g_uavHeap->GetHeap(), L"UAV Descriptor Heap");

            return true;
        }

        void ProcessDeferredRelease(uint32 frameIndex)
        {
            std::lock_guard lock(g_deferredReleaseMutex);

            g_deferredReleaseFlag[frameIndex] = 0;

            g_rtvHeap->ProcessDeferredFree(frameIndex);
            g_dsvHeap->ProcessDeferredFree(frameIndex);
            g_srvHeap->ProcessDeferredFree(frameIndex);
            g_uavHeap->ProcessDeferredFree(frameIndex);

            std::vector<ComPtr<IUnknown>>& resources = g_deferredReleaseResources[frameIndex];
            for (ComPtr<IUnknown>& resource : resources) resource.Reset();
            resources.clear();
        }
        void ReleaseHeapsAndDeferredResources()
        {
            for (uint32 i = 0; i < NUM_FRAME_BUFFERS; ++i)
            {
                if (g_deferredReleaseFlag[i]) ProcessDeferredRelease(i);
            }

            g_rtvHeap->Release();
            g_dsvHeap->Release();
            g_srvHeap->Release();
            g_uavHeap->Release();

            ProcessDeferredRelease(0);
        }
    }

    bool Initialize()
    {
        if (g_device) Shutdown();

        EnableDebugLayer();
        CreateDevice();

        if (!CreateDescriptorHeap()) return false;
        if (!Shader::Initialize()) return false;
        if (!GPass::Initialize()) return false;
        if (!PostProcess::Initialize()) return false;

        g_command = std::make_unique<DX12Command>(*g_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
        if (!Upload::Initialize()) return false;

        return true;
    }
    void Shutdown()
    {
        Upload::Shutdown();
        g_command->Release();

        PostProcess::Shutdown();
        GPass::Shutdown();
        Shader::Shutdown();
        ReleaseHeapsAndDeferredResources();

    #if defined _DEBUG
        ComPtr<ID3D12DebugDevice2> debugDevice;
        Check(g_device.As(&debugDevice));
        g_device.Reset();
        debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
        debugDevice.Reset();
    #endif
        g_factory.Reset();
    }

    ID CreateSurface(ID windowID)
    {
        ID surfaceID = g_surfaces.Emplace(windowID);
        DX12Surface& surface = g_surfaces.Get(surfaceID);
        surface.CreateSwapChain(g_factory.Get(), g_command->GetCommandQueue());
        return surfaceID;
    }
    void RemoveSurface(ID& surfaceID)
    {
        g_command->Flush();
        g_surfaces.Remove(surfaceID);
    }
    void RenderSurface(ID surfaceID)
    {
        g_command->OnFrameBegin();

        const uint32 currFrameIndex = GetCurrFrameIndex();
        if (g_deferredReleaseFlag[currFrameIndex]) ProcessDeferredRelease(currFrameIndex);

        DX12Surface& surface = g_surfaces.Get(surfaceID);

        FrameInfo frameInfo {
            surface.GetWidth(),
            surface.GetHeight(),
        };

        GPass::CheckSize({ frameInfo.surfaceWidth, frameInfo.surfaceHeight });

        ID3D12Resource* backBuffer = surface.GetBackBuffer();

        // Record commands
        {
            GraphicsCommandList* cmdList = g_command->GetCommandList();

            cmdList->RSSetViewports(1, &surface.GetViewport());
            cmdList->RSSetScissorRects(1, &surface.GetScissorRect());

            ID3D12DescriptorHeap* const heaps[] = { g_srvHeap->GetHeap() };
            cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

            // Depth prepass
            GPass::AddTransitionForDepthPrepass(g_barrierContainer);
            g_barrierContainer.Commit(*cmdList);

            GPass::SetRenderTargetForDepthPrepass(cmdList);
            GPass::RenderDepthPrepass(cmdList, frameInfo);

            // Geometry and lighting pass
            GPass::AddTransitionForGPass(g_barrierContainer);
            g_barrierContainer.Commit(*cmdList);

            GPass::SetRenderTargetForGPass(cmdList);
            GPass::Render(cmdList, frameInfo);

            g_barrierContainer.AddTransition(*backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

            // Post-process
            GPass::AddTransitionForPostProcess(g_barrierContainer);
            g_barrierContainer.Commit(*cmdList);

            PostProcess::PostProcess(*cmdList, surface.GetRTV());

            Helper::Barrier::Transition(*cmdList, *backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        }

        g_command->OnFrameEnd(surface);
    }

    void DeferredRelease(ComPtr<IUnknown>& resource)
    {
        if (!resource) return;

        const uint32 frameIndex = GetCurrFrameIndex();
        std::lock_guard lock(g_deferredReleaseMutex);

        g_deferredReleaseResources[frameIndex].push_back(std::move(resource));
        SetDeferredReleaseFlag(frameIndex);
    }
    void SetDeferredReleaseFlag(uint32 frameIndex) { g_deferredReleaseFlag[frameIndex] = 1; }

    Device& GetDevice() { return *g_device.Get(); }
    uint32 GetCurrFrameIndex() { return g_command->GetCurrFrameIndex(); }
    DescriptorHeap& GetRTVHeap() { return *g_rtvHeap; }
    DescriptorHeap& GetDSVHeap() { return *g_dsvHeap; }
    DescriptorHeap& GetSRVHeap() { return *g_srvHeap; }
    DescriptorHeap& GetUAVHeap() { return *g_uavHeap; }
    DXGI_FORMAT GetRenderTargetFormat() { return DX12Surface::DEFAULT_RENDER_TARGET_FORMAT; }

    uint32 GetSurfaceWidth(ID surfaceID) { return g_surfaces.Get(surfaceID).GetWidth(); }
    uint32 GetSurfaceHeight(ID surfaceID) { return g_surfaces.Get(surfaceID).GetHeight(); }
    void ResizeSurface(ID surfaceID)
    {
        g_command->Flush();
        g_surfaces.Get(surfaceID).Resize();
    }
}