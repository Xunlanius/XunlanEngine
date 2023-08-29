#include "DX12RHI.h"
#include "src/Function/Renderer/WindowSystem.h"
#include "DX12Surface.h"
#include "DX12Command.h"
#include "DX12RenderContext.h"
#include "DX12Mesh.h"
#include "DX12Shader.h"
#include "Texture/DX12ImageTexture.h"
#include "DX12DepthStencilState.h"
#include "DX12RasterizerState.h"
#include "DX12CBuffer.h"
#include "DX12Material.h"
#include "DX12RenderItem.h"
#include "DX12RootParameter.h"
#include "Helper/d3dx12.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    DX12RHI::DX12RHI(Platform platform) : RHI(platform)
    {
        EnableDebugLayer();
        CreateDevice();
        CreateDescriptorHeap();

        m_mainCommand = std::make_unique<DX12Command>(GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
        m_uploadContext = std::make_unique<UploadContext>(GetDevice());

        WindowSystem& windowSystem = WindowSystem::Instance();

        const uint32 width = windowSystem.GetWidth();
        const uint32 height = windowSystem.GetHeight();
        const HWND hwnd = (HWND)windowSystem.GetHandle();

        m_surface = std::make_unique<DX12Surface>(hwnd, width, height);
        m_surface->CreateSwapChain(
            m_factory.Get(),
            GetDevice(),
            m_mainCommand->GetCommandQueue(),
            GetRTVHeap()
        );

        CreateRootSignature();
    }

    DX12RHI::~DX12RHI()
    {
        Flush();

        m_psoContainer.clear();
        m_defaultRootSig.Reset();

        m_surface.reset();
        m_mainCommand.reset();
        m_uploadContext.reset();

        ReleaseHeapsAndDeferredResources();

#if defined _DEBUG
        ComPtr<ID3D12DebugDevice2> debugDevice;
        Check(m_device.As(&debugDevice));
        m_device.Reset();
        debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
        debugDevice.Reset();
#endif
        m_factory.Reset();
    }

    void DX12RHI::Flush() { m_mainCommand->Flush(); }

    void DX12RHI::Resize(uint32 width, uint32 height)
    {
        m_mainCommand->Flush();
        m_surface->Resize(width, height);
    }

    Ref<RenderContext> DX12RHI::CreateRenderContext()
    {
        m_mainCommand->BeginRecord();
        GraphicsCommandList* cmdList = m_mainCommand->GetCommandList();

        ID3D12DescriptorHeap* const heaps[] = { GetSRVHeap().GetHeap() };
        cmdList->SetDescriptorHeaps(_countof(heaps), heaps);
        cmdList->SetGraphicsRootSignature(m_defaultRootSig.Get());

        ProcessDeferredRelease(GetCurrFrameIndex());

        return MakeRef<DX12RenderContext>(cmdList);
    }

    void DX12RHI::Execute(Ref<RenderContext>& context)
    {
        m_mainCommand->EndRecord();
        context.reset();
    }

#pragma region RenderTarget

    void DX12RHI::SetRT(Ref<RenderContext> context)
    {
        assert(context);

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        ComPtr<ID3D12Resource> backBuffer = m_surface->GetBackBuffer();
        const D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_surface->GetRTV();

        m_currRTFormats.clear();

        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET)
        );
        cmdList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

        m_currRTFormats.push_back(m_surface->GetFormat());
        m_currDSFormat = DXGI_FORMAT_UNKNOWN;
    }

    void DX12RHI::SetRT(Ref<RenderContext> context, const std::vector<CRef<RenderTarget>>& rts)
    {
        assert(context);
        assert(rts.size() > 0);

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        SetRT(*cmdList, rts, nullptr);
    }

    void DX12RHI::SetRT(Ref<RenderContext> context, CRef<DepthBuffer> depthBuffer)
    {
        assert(context);
        assert(depthBuffer);

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        SetRT(*cmdList, {}, depthBuffer);
    }

    void DX12RHI::SetRT(Ref<RenderContext> context, const std::vector<CRef<RenderTarget>>& rts, CRef<DepthBuffer> depthBuffer)
    {
        assert(context);
        assert(rts.size() > 0);
        assert(depthBuffer);

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        SetRT(*cmdList, rts, depthBuffer);
    }

    void DX12RHI::ClearRT(Ref<RenderContext> context)
    {
        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        cmdList->ClearRenderTargetView(m_surface->GetRTV(), m_clearColor, 0, nullptr);
    }

    void DX12RHI::ClearRT(Ref<RenderContext> context, const std::vector<CRef<RenderTarget>>& rts)
    {
        assert(context);
        assert(rts.size() > 0);

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        ClearRT(*cmdList, rts, nullptr);
    }

    void DX12RHI::ClearRT(Ref<RenderContext> context, CRef<DepthBuffer> depthBuffer)
    {
        assert(context);
        assert(depthBuffer);

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        ClearRT(*cmdList, {}, depthBuffer);
    }

    void DX12RHI::ClearRT(Ref<RenderContext> context, const std::vector<CRef<RenderTarget>>& rts, CRef<DepthBuffer> depthBuffer)
    {
        assert(context);
        assert(rts.size() > 0);
        assert(depthBuffer);

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        ClearRT(*cmdList, rts, depthBuffer);
    }

    void DX12RHI::ResetRT(Ref<RenderContext> context)
    {
        assert(context);

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            m_surface->GetBackBuffer().Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT)
        );
    }

    void DX12RHI::ResetRT(Ref<RenderContext> context, const std::vector<CRef<RenderTarget>>& rts)
    {
        assert(context);
        assert(rts.size());

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        ResetRT(*cmdList, rts, nullptr);
    }

    void DX12RHI::ResetRT(Ref<RenderContext> context, CRef<DepthBuffer> depthBuffer)
    {
        assert(context);
        assert(depthBuffer);

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        ResetRT(*cmdList, {}, depthBuffer);
    }

    void DX12RHI::ResetRT(Ref<RenderContext> context, const std::vector<CRef<RenderTarget>>& rts, CRef<DepthBuffer> depthBuffer)
    {
        assert(context);
        assert(rts.size() > 0);
        assert(depthBuffer);

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        ResetRT(*cmdList, rts, depthBuffer);
    }

#pragma endregion

    void DX12RHI::SetViewport(Ref<RenderContext> context, uint32 x, uint32 y, uint32 width, uint32 height)
    {
        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        m_surface->SetViewport(x, y, width, height);
        cmdList->RSSetViewports(1, &m_surface->GetViewport());
        cmdList->RSSetScissorRects(1, &m_surface->GetScissorRect());
    }

#pragma region CreateResource

    Ref<Mesh> DX12RHI::CreateMesh(const CRef<MeshRawData>& meshRawData)
    {
        return DX12Mesh::Create(meshRawData);
    }
    Ref<Shader> DX12RHI::CreateShader(ShaderType type, const std::filesystem::path& path, const std::string& functionName)
    {
        return DX12Shader::Create(type, path, functionName);
    }
    Ref<ImageTexture> DX12RHI::CreateImageTexture(const CRef<RawTexture>& rawTexture)
    {
        return DX12ImageTexture::Create(rawTexture);
    }
    Ref<RenderTarget> DX12RHI::CreateRT(uint32 width, uint32 height, TextureFormat format)
    {
        return MakeRef<DX12RenderTarget>(width, height, format);
    }
    Ref<DepthBuffer> DX12RHI::CreateDepthBuffer(uint32 width, uint32 height)
    {
        return MakeRef<DX12DepthBuffer>(width, height);
    }
    Ref<RasterizerState> DX12RHI::CreateRasterizerState(const RasterizerStateDesc& desc)
    {
        return MakeRef<DX12RasterizerState>(desc);
    }
    Ref<DepthStencilState> DX12RHI::CreateDepthStencilState()
    {
        return MakeRef<DX12DepthStencilState>();
    }
    Ref<CBuffer> DX12RHI::CreateCBuffer(CBufferType type, uint32 size)
    {
        return MakeRef<DX12CBuffer>(type, size);
    }
    Ref<Material> DX12RHI::CreateMaterial(const std::string& name, MaterialType type, const ShaderList& shaderList)
    {
        return MakeRef<DX12Material>(name, type, shaderList);
    }
    Ref<RenderItem> DX12RHI::CreateRenderItem(const Ref<Mesh>& mesh)
    {
        return MakeRef<DX12RenderItem>(mesh);
    }
    Ref<RenderItem> DX12RHI::CreateRenderItem(const Ref<Mesh>& mesh, const std::vector<Ref<Material>>& materials)
    {
        return MakeRef<DX12RenderItem>(mesh, materials);
    }

#pragma endregion

    void DX12RHI::DeferredRelease(Microsoft::WRL::ComPtr<IUnknown>& resource)
    {
        if (!resource) return;

        const uint32 frameIndex = GetCurrFrameIndex();
        std::lock_guard lock(m_deferredReleaseMutex);

        m_deferredReleaseResources[frameIndex].push_back(std::move(resource));
        SetDeferredReleaseFlag(frameIndex);
    }

    void DX12RHI::EnableDebugLayer()
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
    
    void DX12RHI::CreateDevice()
    {
        //ID3D12Device指负责渲染的对象，我们需要指定硬件来创建device
        //IDXGIAdapter指显示适配器，可通过IDXGIFactory的EnumAdapters函数来获取

        uint32 factoryFlags = 0;
    #if defined _DEBUG
        factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    #endif

        Check(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)));

        ComPtr<IDXGIAdapter4> targetAdapter;
        ComPtr<IDXGIAdapter4> currAdapter;
        uint64 maxDedicatedVideoMemory = 0;

        //若索引值超出系统中adapter的个数，EnumAdapters会返回DXGI_ERROR_NOT_FOUND
        for (uint32 i = 0;
            m_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&currAdapter)) != DXGI_ERROR_NOT_FOUND;
            ++i)
        {
            DXGI_ADAPTER_DESC1 desc = {};
            currAdapter->GetDesc1(&desc);

            //将adapter的独占显存作为评价adapter好坏的标准
            if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && maxDedicatedVideoMemory < desc.DedicatedVideoMemory)
            {
                maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
                targetAdapter = currAdapter;
            }
        }

        //pAdapter：显示适配器（GPU）的指针，若为空指针，则为默认的主显示适配器（一般是默认使用的独立显卡）
        //MinimumFeatureLevel：要求该显示适配器最低支持的DirectX的版本。若无法支持该版本,该函数会返回创建失败的异常报告
        Check(D3D12CreateDevice(targetAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
        NAME_OBJECT(m_device, L"Main Device");
    }
    
    void DX12RHI::CreateDescriptorHeap()
    {
        m_rtvHeap = std::make_unique<DX12DescriptorHeap>(GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 512, false);
        m_dsvHeap = std::make_unique<DX12DescriptorHeap>(GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 512, false);
        m_srvHeap = std::make_unique<DX12DescriptorHeap>(GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096, true);

        NAME_OBJECT(m_rtvHeap->GetHeap(), L"RTV Descriptor Heap");
        NAME_OBJECT(m_dsvHeap->GetHeap(), L"DSV Descriptor Heap");
        NAME_OBJECT(m_srvHeap->GetHeap(), L"SRV Descriptor Heap");
    }
    
    void DX12RHI::CreateRootSignature()
    {
        CD3DX12_ROOT_PARAMETER1 params[(uint32)RootParam::Count] = {};
        params[(uint32)RootParam::PerObject].InitAsConstantBufferView(0);
        params[(uint32)RootParam::PerMaterial].InitAsConstantBufferView(1);
        params[(uint32)RootParam::PerFrame].InitAsConstantBufferView(2);
        params[(uint32)RootParam::VertexBuffer].InitAsShaderResourceView(0);
        params[(uint32)RootParam::GBuffer].InitAsConstantBufferView(3);
        params[(uint32)RootParam::ShadowMaps].InitAsConstantBufferView(4);

        const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> samplers = GetStaticSamplers();

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc = {};
        desc.Init_1_1(
            _countof(params),
            params,
            (uint32)samplers.size(),
            samplers.data(),
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
            D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | 
            D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
        );

        ComPtr<ID3DBlob> sigBlob;
        ComPtr<ID3DBlob> errorBlob;
        if (FAILED(D3D12SerializeVersionedRootSignature(&desc, &sigBlob, &errorBlob)))
        {
            OutputDebugStringA(errorBlob ? (char*)errorBlob->GetBufferPointer() : "");
            return;
        }

        Check(m_device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_defaultRootSig)));
        NAME_OBJECT(m_defaultRootSig, L"Default Root Signature");
    }

    std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> DX12RHI::GetStaticSamplers() const
    {
        //过滤器POINT,寻址模式WRAP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC pointWarp(0,	//着色器寄存器
            D3D12_FILTER_MIN_MAG_MIP_POINT,		//过滤器类型为POINT(常量插值)
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//U方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//V方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);	//W方向上的寻址模式为WRAP（重复寻址模式）

        //过滤器POINT,寻址模式CLAMP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC pointClamp(1,	//着色器寄存器
            D3D12_FILTER_MIN_MAG_MIP_POINT,		//过滤器类型为POINT(常量插值)
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//U方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//V方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	//W方向上的寻址模式为CLAMP（钳位寻址模式）

        //过滤器LINEAR,寻址模式WRAP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC linearWarp(2,	//着色器寄存器
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,		//过滤器类型为LINEAR(线性插值)
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//U方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//V方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);	//W方向上的寻址模式为WRAP（重复寻址模式）

        //过滤器LINEAR,寻址模式CLAMP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC linearClamp(3,	//着色器寄存器
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,		//过滤器类型为LINEAR(线性插值)
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//U方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//V方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	//W方向上的寻址模式为CLAMP（钳位寻址模式）

        //过滤器ANISOTROPIC,寻址模式WRAP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC anisotropicWarp(4,	//着色器寄存器
            D3D12_FILTER_ANISOTROPIC,			//过滤器类型为ANISOTROPIC(各向异性)
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//U方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//V方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);	//W方向上的寻址模式为WRAP（重复寻址模式）

        //过滤器LINEAR,寻址模式CLAMP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(5,	//着色器寄存器
            D3D12_FILTER_ANISOTROPIC,			//过滤器类型为ANISOTROPIC(各向异性)
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//U方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//V方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	//W方向上的寻址模式为CLAMP（钳位寻址模式）

        return { pointWarp, pointClamp, linearWarp, linearClamp, anisotropicWarp, anisotropicClamp };
    }

    void DX12RHI::ProcessDeferredRelease(uint32 frameIndex)
    {
        if (m_deferredReleaseFlag[frameIndex] = 0) return;

        std::lock_guard lock(m_deferredReleaseMutex);

        m_deferredReleaseFlag[frameIndex] = 0;

        m_rtvHeap->ProcessDeferredFree(frameIndex);
        m_dsvHeap->ProcessDeferredFree(frameIndex);
        m_srvHeap->ProcessDeferredFree(frameIndex);

        std::vector<ComPtr<IUnknown>>& resources = m_deferredReleaseResources[frameIndex];
        for (ComPtr<IUnknown>& resource : resources)
        {
            resource.Reset();
        }
        resources.clear();
    }
    
    void DX12RHI::ReleaseHeapsAndDeferredResources()
    {
        for (uint32 i = 0; i < NUM_FRAME_BUFFERS; ++i)
        {
            if (m_deferredReleaseFlag[i]) ProcessDeferredRelease(i);
        }

        m_rtvHeap.reset();
        m_dsvHeap.reset();
        m_srvHeap.reset();
    }

    void DX12RHI::SetRT(GraphicsCommandList& cmdList, const std::vector<CRef<RenderTarget>>& rts, CRef<DepthBuffer> depthBuffer)
    {
        m_currRTFormats.clear();

        std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};

        for (auto& rt : rts)
        {
            const CRef<DX12RenderTarget> dx12RT = std::dynamic_pointer_cast<const DX12RenderTarget>(rt);
            assert(dx12RT);

            rtvs.push_back(dx12RT->GetRTV().handleCPU);

            barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
                dx12RT->GetResource(),
                dx12RT->RT_INIT_STATE,
                D3D12_RESOURCE_STATE_RENDER_TARGET)
            );

            m_currRTFormats.push_back(dx12RT->GetDXFormat());
        }

        if (depthBuffer)
        {
            const CRef<DX12DepthBuffer> dx12DepthBuffer = std::dynamic_pointer_cast<const DX12DepthBuffer>(depthBuffer);

            m_currDSFormat = dx12DepthBuffer->GetDXFormat();
            dsv = dx12DepthBuffer->GetDSV().handleCPU;

            barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
                dx12DepthBuffer->GetResource(),
                dx12DepthBuffer->DS_INIT_STATE,
                D3D12_RESOURCE_STATE_DEPTH_WRITE
            ));
        }
        else
        {
            m_currDSFormat = DXGI_FORMAT_UNKNOWN;
        }

        assert(barriers.size() > 0);

        cmdList.ResourceBarrier((uint32)barriers.size(), barriers.data());
        cmdList.OMSetRenderTargets(
            (uint32)rtvs.size(),
            (rtvs.size() > 0) ? rtvs.data() : nullptr,
            FALSE,
            depthBuffer ? &dsv : nullptr
        );
    }
    
    void DX12RHI::ClearRT(GraphicsCommandList& cmdList, const std::vector<CRef<RenderTarget>>& rts, CRef<DepthBuffer> depthBuffer)
    {
        for (auto& rt : rts)
        {
            const CRef<DX12RenderTarget> dx12RT = std::dynamic_pointer_cast<const DX12RenderTarget>(rt);
            assert(dx12RT);

            cmdList.ClearRenderTargetView(dx12RT->GetRTV().handleCPU, m_clearColor, 0, nullptr);
        }

        if (depthBuffer)
        {
            const CRef<DX12DepthBuffer> dx12DepthBuffer = std::dynamic_pointer_cast<const DX12DepthBuffer>(depthBuffer);

            cmdList.ClearDepthStencilView(
                dx12DepthBuffer->GetDSV().handleCPU,
                D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                1.0f, 0,
                0, nullptr
            );
        }
    }
    
    void DX12RHI::ResetRT(GraphicsCommandList& cmdList, const std::vector<CRef<RenderTarget>>& rts, CRef<DepthBuffer> depthBuffer)
    {
        std::vector<CD3DX12_RESOURCE_BARRIER> barriers;

        for (auto& rt : rts)
        {
            const CRef<DX12RenderTarget> dx12RT = std::dynamic_pointer_cast<const DX12RenderTarget>(rt);
            assert(dx12RT);

            barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
                dx12RT->GetResource(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                dx12RT->RT_INIT_STATE)
            );
        }

        if (depthBuffer)
        {
            const CRef<DX12DepthBuffer> dx12DepthBuffer = std::dynamic_pointer_cast<const DX12DepthBuffer>(depthBuffer);

            barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
                dx12DepthBuffer->GetResource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                dx12DepthBuffer->DS_INIT_STATE)
            );
        }

        assert(barriers.size() > 0);

        cmdList.ResourceBarrier((uint32)barriers.size(), barriers.data());
    }
}