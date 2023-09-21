#include "DX12Shader.h"
#include "../DX12RHI.h"
#include <numeric>

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    bool operator==(const DX12PSO& lhs, const DX12PSO& rhs)
    {
        return
            lhs.m_vs.pShaderBytecode == rhs.m_vs.pShaderBytecode &&
            lhs.m_vs.BytecodeLength == rhs.m_vs.BytecodeLength &&
            lhs.m_ps.pShaderBytecode == rhs.m_ps.pShaderBytecode &&
            lhs.m_ps.BytecodeLength == rhs.m_ps.BytecodeLength &&
            *lhs.m_rasterizerState == *rhs.m_rasterizerState &&
            *lhs.m_depthStencilState == *rhs.m_depthStencilState &&
            lhs.m_rtFormats == rhs.m_rtFormats &&
            lhs.m_dsFormat == rhs.m_dsFormat;
    }

    size_t DX12PSO::Hash::operator()(const DX12PSO& key) const
    {
        size_t result = 0;
        result += (size_t)key.m_vs.pShaderBytecode;
        result += key.m_vs.BytecodeLength;
        result += (size_t)key.m_ps.pShaderBytecode;
        result += key.m_ps.BytecodeLength;

        result += DX12RasterizerState::Hash()(*key.m_rasterizerState);
        result += DX12DepthStencilState::Hash()(*key.m_depthStencilState);
        result += std::accumulate(key.m_rtFormats.begin(), key.m_rtFormats.end(), 0);
        result += key.m_dsFormat;
        return result;
    }

    DX12Shader::DX12Shader(const std::string& name, const ShaderInitDesc& desc, const std::filesystem::path& path)
        : Shader(name, desc)
    {
        assert(desc.m_createVS);

        ShaderCompiler compiler;

        if (desc.m_createVS)
        {
            bool succeed = compiler.Compile(path, L"VS", L"vs_6_6");
            assert(succeed);

            StoreByteCode(DX12ShaderType::VS, compiler.GetCompiledShader());
            StoreParam(DX12ShaderType::VS, compiler.GetReflection());
        }

        if (desc.m_createPS)
        {
            bool succeed = compiler.Compile(path, L"PS", L"ps_6_6");
            assert(succeed);

            StoreByteCode(DX12ShaderType::PS, compiler.GetCompiledShader());
            StoreParam(DX12ShaderType::PS, compiler.GetReflection());
        }

        CreateRootSig();
    }

    bool DX12Shader::HasParam(const std::string& name) const
    {
        return FindParam(m_paramCBVs, name) != m_paramCBVs.end();
    }

    bool DX12Shader::SetParam(const std::string& name, CRef<CBuffer> cBuffer)
    {
        CRef<DX12CBuffer> dx12CBuffer = CastTo<const DX12CBuffer>(cBuffer);

        bool found = false;

        for (auto& param : m_paramCBVs)
        {
            if (param.m_name == name)
            {
                param.m_gpuAddress = dx12CBuffer->GetGPUAddress();
                found = true;
            }
        }

        return found;
    }

    bool DX12Shader::SetParam(const std::string& name, CRef<DataBuffer> buffer)
    {
        CRef<DX12DataBuffer> dx12DataBuffer = CastTo<const DX12DataBuffer>(buffer);

        bool found = false;

        for (auto& param : m_paramSRVs)
        {
            if (param.m_name == name)
            {
                param.m_gpuAddress = dx12DataBuffer->GetGPUAddress();
                found = true;
            }
        }

        return found;
    }

    void DX12Shader::Apply(Ref<RenderContext> context)
    {
        Ref<DX12RenderContext> dx12Context = CastTo<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        // Set PSO and root signature
        cmdList->SetPipelineState(TryGetPSO().Get());
        cmdList->SetGraphicsRootSignature(GetRootSig());

        Bind(context);
    }

    void DX12Shader::Bind(Ref<RenderContext> context)
    {
        CheckBinding();

        Ref<DX12RenderContext> dx12Context = std::dynamic_pointer_cast<DX12RenderContext>(context);
        GraphicsCommandList* cmdList = dx12Context->m_cmdList;

        for (uint32 i = 0; i < m_paramCBVs.size(); i++)
        {
            const uint32 rootParamIndex = m_indexBaseCBV + i;
            const D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_paramCBVs[i].m_gpuAddress;

            cmdList->SetGraphicsRootConstantBufferView(rootParamIndex, gpuAddress);
        }

        for (uint32 i = 0; i < m_paramSRVs.size(); i++)
        {
            const uint32 rootParamIndex = m_indexBaseSRV + i;
            const D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_paramSRVs[i].m_gpuAddress;

            cmdList->SetGraphicsRootShaderResourceView(rootParamIndex, gpuAddress);
        }

        for (uint32 i = 0; i < m_paramUAVs.size(); i++)
        {
            const uint32 rootParamIndex = m_indexBaseUAV + i;
            const D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_paramUAVs[i].m_gpuAddress;

            cmdList->SetGraphicsRootUnorderedAccessView(rootParamIndex, gpuAddress);
        }

        ClearBinding();
    }

    D3D12_SHADER_BYTECODE DX12Shader::GetByteCode(DX12ShaderType type) const
    {
        auto it = m_shaderMap.find(type);
        assert(it != m_shaderMap.end());

        return { it->second.data(), it->second.size() };
    }

    void DX12Shader::StoreByteCode(DX12ShaderType type, Microsoft::WRL::ComPtr<IDxcBlob> compiledShader)
    {
        const void* byteCode = compiledShader->GetBufferPointer();
        const size_t byteCodeLength = compiledShader->GetBufferSize();

        assert(m_shaderMap.find(type) == m_shaderMap.end());

        std::vector<byte>& code = m_shaderMap[type];
        code.resize(byteCodeLength);
        memcpy(code.data(), byteCode, byteCodeLength);
    }

    void DX12Shader::StoreParam(DX12ShaderType type, Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflection)
    {
        D3D12_SHADER_DESC shaderDesc = {};
        reflection->GetDesc(&shaderDesc);

        for (uint32 i = 0; i < shaderDesc.BoundResources; i++)
        {
            D3D12_SHADER_INPUT_BIND_DESC  bindingDesc = {};
            reflection->GetResourceBindingDesc(i, &bindingDesc);

            const char* varName = bindingDesc.Name;
            uint32 bindPoint = bindingDesc.BindPoint;
            uint32 bindCount = bindingDesc.BindCount;
            uint32 registerSpace = bindingDesc.Space;
            D3D_SHADER_INPUT_TYPE resourceType = bindingDesc.Type;

            std::cout << "    Name: " << varName << std::endl;
            std::cout << "    Bind Point: " << bindPoint << std::endl;
            std::cout << "    Bind Count: " << bindCount << std::endl;
            std::cout << "    Register Space: " << registerSpace << std::endl;
            std::cout << "    Resource Type: " << resourceType << std::endl;
            std::cout << std::endl;

            switch (resourceType)
            {
            case D3D_SIT_CBUFFER:
            {
                CBVParam param = {};
                param.m_shaderType = type;
                param.m_name = varName;
                param.m_bindPoint = bindPoint;
                param.m_bindCount = bindCount;
                param.m_registerSpace = registerSpace;

                auto it = FindParam(m_paramCBVs, varName, bindPoint);

                if (it == m_paramCBVs.end())
                {
                    m_paramCBVs.push_back(param);
                }
            }
            break;
            case D3D_SIT_STRUCTURED:
            {
                SRVParam param = {};
                param.m_shaderType = type;
                param.m_name = varName;
                param.m_bindPoint = bindPoint;
                param.m_bindCount = bindCount;
                param.m_registerSpace = registerSpace;

                auto it = FindParam(m_paramSRVs, varName, bindPoint);

                if (it == m_paramSRVs.end())
                {
                    m_paramSRVs.push_back(param);
                }
            }
            break;
            case D3D_SIT_UAV_RWSTRUCTURED:
            {
                UAVParam param = {};
                param.m_shaderType = type;
                param.m_name = varName;
                param.m_bindPoint = bindPoint;
                param.m_bindCount = bindCount;
                param.m_registerSpace = registerSpace;

                auto it = FindParam(m_paramUAVs, varName, bindPoint);

                if (it == m_paramUAVs.end())
                {
                    m_paramUAVs.push_back(param);
                }
            }
            break;
            case D3D_SIT_SAMPLER: break;
            case D3D_SIT_TEXTURE:
            case D3D_SIT_UAV_RWTYPED:
            default: assert(false);
            }
        }
    }

    void DX12Shader::CreateRootSig()
    {
        std::vector<CD3DX12_ROOT_PARAMETER1> rootParams;

        for (const CBVParam& param : m_paramCBVs)
        {
            if (m_indexBaseCBV == -1)
            {
                m_indexBaseCBV = (int)rootParams.size();
            }

            CD3DX12_ROOT_PARAMETER1& rootParam = rootParams.emplace_back();
            rootParam.InitAsConstantBufferView(param.m_bindPoint, param.m_registerSpace);
        }

        for (const SRVParam& param : m_paramSRVs)
        {
            if (m_indexBaseSRV == -1)
            {
                m_indexBaseSRV = (int)rootParams.size();
            }

            CD3DX12_ROOT_PARAMETER1& rootParam = rootParams.emplace_back();
            rootParam.InitAsShaderResourceView(param.m_bindPoint, param.m_registerSpace);
        }

        for (const UAVParam& param : m_paramUAVs)
        {
            if (m_indexBaseUAV == -1)
            {
                m_indexBaseUAV = (int)rootParams.size();
            }

            CD3DX12_ROOT_PARAMETER1& rootParam = rootParams.emplace_back();
            rootParam.InitAsUnorderedAccessView(param.m_bindPoint, param.m_registerSpace);
        }

        const std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplers = GetStaticSamplers();

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc = {};
        desc.Init_1_1(
            (uint32)rootParams.size(),
            rootParams.data(),
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
            if (errorBlob)
            {
                std::cout << (char*)errorBlob->GetBufferPointer() << std::endl;
            }
            return;
        }

        Device& device = DX12RHI::Instance().GetDevice();

        Check(device.CreateRootSignature(
            0,
            sigBlob->GetBufferPointer(),
            sigBlob->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSig)
        ));
    }

    std::vector<CD3DX12_STATIC_SAMPLER_DESC> DX12Shader::GetStaticSamplers()
    {
        //过滤器POINT,寻址模式WRAP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC pointWarp(
            0,                                  //着色器寄存器
            D3D12_FILTER_MIN_MAG_MIP_POINT,     //过滤器类型为POINT(常量插值)
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,    //U方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,    //V方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP     //W方向上的寻址模式为WRAP（重复寻址模式）
        );

        //过滤器POINT,寻址模式CLAMP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC pointClamp(
            1,                                  //着色器寄存器
            D3D12_FILTER_MIN_MAG_MIP_POINT,     //过滤器类型为POINT(常量插值)
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,   //U方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,   //V方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP    //W方向上的寻址模式为CLAMP（钳位寻址模式）
        );

        //过滤器LINEAR,寻址模式WRAP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC linearWarp(
            2,                                  //着色器寄存器
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,    //过滤器类型为LINEAR(线性插值)
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,    //U方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,    //V方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP     //W方向上的寻址模式为WRAP（重复寻址模式）
        );

        //过滤器LINEAR,寻址模式CLAMP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC linearClamp(
            3,                                  //着色器寄存器
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,    //过滤器类型为LINEAR(线性插值)
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,   //U方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,   //V方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP    //W方向上的寻址模式为CLAMP（钳位寻址模式）
        );

        //过滤器ANISOTROPIC,寻址模式WRAP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC anisotropicWarp(
            4,                                  //着色器寄存器
            D3D12_FILTER_ANISOTROPIC,           //过滤器类型为ANISOTROPIC(各向异性)
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,    //U方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,    //V方向上的寻址模式为WRAP（重复寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_WRAP     //W方向上的寻址模式为WRAP（重复寻址模式）
        );

        //过滤器LINEAR,寻址模式CLAMP的静态采样器
        CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
            5,                                  //着色器寄存器
            D3D12_FILTER_ANISOTROPIC,           //过滤器类型为ANISOTROPIC(各向异性)
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,   //U方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,   //V方向上的寻址模式为CLAMP（钳位寻址模式）
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP    //W方向上的寻址模式为CLAMP（钳位寻址模式）
        );

        return { pointWarp, pointClamp, linearWarp, linearClamp, anisotropicWarp, anisotropicClamp };
    }

    void DX12Shader::CheckBinding() const
    {
        for (const CBVParam& param : m_paramCBVs)
        {
            assert(param.m_gpuAddress > 0);
        }

        for (const SRVParam& param : m_paramSRVs)
        {
            assert(param.m_gpuAddress > 0);
        }

        for (const UAVParam& param : m_paramUAVs)
        {
            assert(param.m_gpuAddress > 0);
        }
    }

    void DX12Shader::ClearBinding()
    {
        for (CBVParam& param : m_paramCBVs)
        {
            param.m_gpuAddress = 0;
        }

        for (SRVParam& param : m_paramSRVs)
        {
            param.m_gpuAddress = 0;
        }

        for (UAVParam& param : m_paramUAVs)
        {
            param.m_gpuAddress = 0;
        }
    }

    Microsoft::WRL::ComPtr<ID3D12PipelineState> DX12Shader::TryGetPSO() const
    {
        const DX12PSO psoDesc = BuildPSODesc();

        auto& psoContainer = DX12RHI::Instance().GetPSOContainer();

        auto it = psoContainer.find(psoDesc);
        if (it != psoContainer.end())
        {
            return it->second;
        }
        else
        {
            auto [newOne, succeed] = psoContainer.emplace(psoDesc, CreatePSO(psoDesc));
            return newOne->second;
        }
    }

    Microsoft::WRL::ComPtr<ID3D12PipelineState> DX12Shader::CreatePSO(const DX12PSO& psoDesc) const
    {
        DX12RHI& rhi = DX12RHI::Instance();
        Device& device = rhi.GetDevice();

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = GetRootSig();
        desc.VS = psoDesc.m_vs;
        desc.PS = psoDesc.m_ps;
        desc.BlendState = Helper::Blend::DISABLED;
        desc.SampleMask = UINT32_MAX; // It is neccessary. Without it, nothing will be rendered.
        desc.RasterizerState = psoDesc.m_rasterizerState->GetDX12Desc();
        desc.DepthStencilState = psoDesc.m_depthStencilState->GetDX12Desc();
        //desc.InputLayout = rhi.GetInputLayout();
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets = std::min((uint32)psoDesc.m_rtFormats.size(), 8u);
        if (desc.NumRenderTargets > 0)
        {
            const size_t byteSize = desc.NumRenderTargets * sizeof(DXGI_FORMAT);
            std::memcpy(desc.RTVFormats, psoDesc.m_rtFormats.data(), byteSize);
        }
        desc.DSVFormat = psoDesc.m_dsFormat;
        desc.SampleDesc = { 1, 0 };

        ComPtr<ID3D12PipelineState> pso;
        Check(device.CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso)));
        NAME_OBJECT_INDEX(pso, L"PSO - key: ", DX12PSO::Hash()(psoDesc));

        return pso;
    }
    
    DX12PSO DX12Shader::BuildPSODesc() const
    {
        DX12RHI& rhi = DX12RHI::Instance();
        CRef<DX12RasterizerState> rasterizerState = CastTo<const DX12RasterizerState>(m_rasterizerState);
        CRef<DX12DepthStencilState> depthStencilState = CastTo<const DX12DepthStencilState>(m_depthStencilState);

        DX12PSO psoDesc = {};
        if (ContainVS()) psoDesc.m_vs = GetByteCode(DX12ShaderType::VS);
        if (ContainPS()) psoDesc.m_ps = GetByteCode(DX12ShaderType::PS);
        psoDesc.m_rasterizerState = rasterizerState;
        psoDesc.m_depthStencilState = depthStencilState;
        psoDesc.m_rtFormats = rhi.GetRTFormats();
        psoDesc.m_dsFormat = rhi.GetDSFormat();

        return psoDesc;
    }
}