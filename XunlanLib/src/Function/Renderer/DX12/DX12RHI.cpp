#include "DX12RHI.h"
#include "src/Function/Renderer/WindowSystem.h"
#include "DX12Surface.h"
#include "DX12Command.h"
#include "DX12RenderContext.h"
#include "DX12Mesh.h"
#include "DX12Shader.h"
#include "DX12ImageTexture.h"
#include "DX12RenderTarget.h"
#include "DX12DepthStencilState.h"
#include "DX12RasterizerState.h"
#include "DX12CBuffer.h"
#include "DX12Material.h"
#include "DX12RenderItem.h"
#include "DX12RootParameter.h"
#include "d3dx12.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    DX12RHI::DX12RHI(Platform platform, const RHIInitDesc& initDesc) : RHI(platform, initDesc)
    {
        EnableDebugLayer();
        CreateDevice();
        CreateDescriptorHeap();

        m_mainCommand = std::make_unique<DX12Command>(GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
        m_uploadContext = std::make_unique<UploadContext>(GetDevice());

        const uint32 width = initDesc.windowSystem->GetWidth();
        const uint32 height = initDesc.windowSystem->GetHeight();
        const HWND hwnd = (HWND)initDesc.windowSystem->GetHandle();

        m_surface = std::make_unique<DX12Surface>(hwnd, width, height);
        m_surface->CreateSwapChain(m_factory.Get(), GetDevice(), m_mainCommand->GetCommandQueue(), GetRTVHeap());

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

    void DX12RHI::SetRenderTarget(const Ref<RenderContext>& context, const CRef<RenderTarget>& renderTarget)
    {
        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;
        
        if (renderTarget)
        {
            const CRef<DX12RenderTarget> dx12RT = std::dynamic_pointer_cast<const DX12RenderTarget>(renderTarget);
            const DescriptorHandle rtv = dx12RT->GetRTV();
            const DescriptorHandle dsv = dx12RT->GetDSV();

            if (rtv.IsValid())
            {
                const CD3DX12_RESOURCE_BARRIER barriers[2] =
                {
                    CD3DX12_RESOURCE_BARRIER::Transition(dx12RT->GetRT(),
                        dx12RT->RT_INIT_STATE,
                        D3D12_RESOURCE_STATE_RENDER_TARGET
                    ),
                    CD3DX12_RESOURCE_BARRIER::Transition(dx12RT->GetDS(),
                        dx12RT->DS_INIT_STATE,
                        D3D12_RESOURCE_STATE_DEPTH_WRITE
                    ),
                };

                cmdList->ResourceBarrier(_countof(barriers), barriers);

                cmdList->OMSetRenderTargets(1, &rtv.handleCPU, FALSE, &dsv.handleCPU);
                m_currRTFormat = dx12RT->GetRTFormat();
                m_currDSFormat = dx12RT->GetDSFormat();
            }
            else
            {
                cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                    dx12RT->GetDS(),
                    dx12RT->DS_INIT_STATE,
                    D3D12_RESOURCE_STATE_DEPTH_WRITE)
                );

                cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsv.handleCPU);
                m_currRTFormat = DXGI_FORMAT_UNKNOWN;
                m_currDSFormat = dx12RT->GetDSFormat();
            }
        }
        else
        {
            ComPtr<ID3D12Resource> backBuffer = m_surface->GetBackBuffer();
            const D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_surface->GetRTV();

            cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                backBuffer.Get(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET)
            );
            cmdList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
            m_currRTFormat = m_surface->GetFormat();
            m_currDSFormat = DXGI_FORMAT_UNKNOWN;
        }
    }
    void DX12RHI::ClearRenderTarget(const Ref<RenderContext>& context, const CRef<RenderTarget>& renderTarget)
    {
        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        if (renderTarget)
        {
            const CRef<DX12RenderTarget> dx12RT = std::dynamic_pointer_cast<const DX12RenderTarget>(renderTarget);
            const DescriptorHandle rtv = dx12RT->GetRTV();
            const DescriptorHandle dsv = dx12RT->GetDSV();

            if (rtv.IsValid()) cmdList->ClearRenderTargetView(rtv.handleCPU, clearColor, 0, nullptr);
            cmdList->ClearDepthStencilView(dsv.handleCPU, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        }
        else
        {
            cmdList->ClearRenderTargetView(m_surface->GetRTV(), DirectX::Colors::SkyBlue, 0, nullptr);
        }
    }
    void DX12RHI::ResetRenderTarget(const Ref<RenderContext>& context, const CRef<RenderTarget>& renderTarget)
    {
        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        if (renderTarget)
        {
            const CRef<DX12RenderTarget> dx12RT = std::dynamic_pointer_cast<const DX12RenderTarget>(renderTarget);
            const DescriptorHandle rtv = dx12RT->GetRTV();
            const DescriptorHandle dsv = dx12RT->GetDSV();

            if (rtv.IsValid())
            {
                cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                    dx12RT->GetRT(),
                    D3D12_RESOURCE_STATE_RENDER_TARGET,
                    dx12RT->RT_INIT_STATE)
                );
            }

            cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                dx12RT->GetDS(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                dx12RT->DS_INIT_STATE)
            );
        }
        else
        {
            cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                m_surface->GetBackBuffer().Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT)
            );
        }
    }
    void DX12RHI::SetViewport(const Ref<RenderContext>& context, uint32 x, uint32 y, uint32 width, uint32 height)
    {
        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        m_surface->SetViewport(x, y, width, height);
        cmdList->RSSetViewports(1, &m_surface->GetViewport());
        cmdList->RSSetScissorRects(1, &m_surface->GetScissorRect());
    }

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
    Ref<RenderTarget> DX12RHI::CreateRenderTarget(uint32 width, uint32 height, RenderTargetUsage usage)
    {
        return DX12RenderTarget::Create(width, height, usage);
    }
    Ref<RasterizerState> DX12RHI::CreateRasterizerState(const RasterizerStateDesc& desc)
    {
        return MakeRef<DX12RasterizerState>(desc);
    }
    Ref<DepthStencilState> DX12RHI::CreateDepthStencilState()
    {
        return MakeRef<DX12DepthStencilState>();
    }
    Ref<CBuffer> DX12RHI::CreateConstantBuffer(CBufferType type, uint32 size)
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
        //ID3D12Deviceָ������Ⱦ�Ķ���������Ҫָ��Ӳ��������device
        //IDXGIAdapterָ��ʾ����������ͨ��IDXGIFactory��EnumAdapters��������ȡ

        uint32 factoryFlags = 0;
    #if defined _DEBUG
        factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    #endif

        Check(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)));

        ComPtr<IDXGIAdapter4> targetAdapter;
        ComPtr<IDXGIAdapter4> currAdapter;
        uint64 maxDedicatedVideoMemory = 0;

        //������ֵ����ϵͳ��adapter�ĸ�����EnumAdapters�᷵��DXGI_ERROR_NOT_FOUND
        for (uint32 i = 0;
            m_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&currAdapter)) != DXGI_ERROR_NOT_FOUND;
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
        CD3DX12_ROOT_PARAMETER1 params[(uint32)DefaultRootParam::COUNT] = {};
        params[(uint32)DefaultRootParam::PER_OBJECT].InitAsConstantBufferView(0);
        params[(uint32)DefaultRootParam::PER_MATERIAL].InitAsConstantBufferView(1);
        params[(uint32)DefaultRootParam::PER_FRAME].InitAsConstantBufferView(2);
        params[(uint32)DefaultRootParam::VERTEX_BUFFER].InitAsShaderResourceView(0);
        params[(uint32)DefaultRootParam::SHADOW_MAP_INDICES].InitAsConstantBufferView(3);
        params[(uint32)DefaultRootParam::TEXTURE_INDICES].InitAsConstantBufferView(4);

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
        //������POINT,ѰַģʽWRAP�ľ�̬������
        CD3DX12_STATIC_SAMPLER_DESC pointWarp(0,	//��ɫ���Ĵ���
            D3D12_FILTER_MIN_MAG_MIP_POINT,		//����������ΪPOINT(������ֵ)
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//U�����ϵ�ѰַģʽΪWRAP���ظ�Ѱַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//V�����ϵ�ѰַģʽΪWRAP���ظ�Ѱַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);	//W�����ϵ�ѰַģʽΪWRAP���ظ�Ѱַģʽ��

        //������POINT,ѰַģʽCLAMP�ľ�̬������
        CD3DX12_STATIC_SAMPLER_DESC pointClamp(1,	//��ɫ���Ĵ���
            D3D12_FILTER_MIN_MAG_MIP_POINT,		//����������ΪPOINT(������ֵ)
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//U�����ϵ�ѰַģʽΪCLAMP��ǯλѰַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//V�����ϵ�ѰַģʽΪCLAMP��ǯλѰַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	//W�����ϵ�ѰַģʽΪCLAMP��ǯλѰַģʽ��

        //������LINEAR,ѰַģʽWRAP�ľ�̬������
        CD3DX12_STATIC_SAMPLER_DESC linearWarp(2,	//��ɫ���Ĵ���
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,		//����������ΪLINEAR(���Բ�ֵ)
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//U�����ϵ�ѰַģʽΪWRAP���ظ�Ѱַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//V�����ϵ�ѰַģʽΪWRAP���ظ�Ѱַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);	//W�����ϵ�ѰַģʽΪWRAP���ظ�Ѱַģʽ��

        //������LINEAR,ѰַģʽCLAMP�ľ�̬������
        CD3DX12_STATIC_SAMPLER_DESC linearClamp(3,	//��ɫ���Ĵ���
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,		//����������ΪLINEAR(���Բ�ֵ)
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//U�����ϵ�ѰַģʽΪCLAMP��ǯλѰַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//V�����ϵ�ѰַģʽΪCLAMP��ǯλѰַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	//W�����ϵ�ѰַģʽΪCLAMP��ǯλѰַģʽ��

        //������ANISOTROPIC,ѰַģʽWRAP�ľ�̬������
        CD3DX12_STATIC_SAMPLER_DESC anisotropicWarp(4,	//��ɫ���Ĵ���
            D3D12_FILTER_ANISOTROPIC,			//����������ΪANISOTROPIC(��������)
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//U�����ϵ�ѰַģʽΪWRAP���ظ�Ѱַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,	//V�����ϵ�ѰַģʽΪWRAP���ظ�Ѱַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);	//W�����ϵ�ѰַģʽΪWRAP���ظ�Ѱַģʽ��

        //������LINEAR,ѰַģʽCLAMP�ľ�̬������
        CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(5,	//��ɫ���Ĵ���
            D3D12_FILTER_ANISOTROPIC,			//����������ΪANISOTROPIC(��������)
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//U�����ϵ�ѰַģʽΪCLAMP��ǯλѰַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//V�����ϵ�ѰַģʽΪCLAMP��ǯλѰַģʽ��
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	//W�����ϵ�ѰַģʽΪCLAMP��ǯλѰַģʽ��

        return { pointWarp, pointClamp, linearWarp, linearClamp, anisotropicWarp, anisotropicClamp };
    }
    /*D3D12_INPUT_LAYOUT_DESC DX12RHI::GetInputLayout() const
    {
        D3D12_INPUT_LAYOUT_DESC inputLayout = {};
        inputLayout.pInputElementDescs = m_elements;
        inputLayout.NumElements = _countof(m_elements);

        return inputLayout;
    }*/

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
}