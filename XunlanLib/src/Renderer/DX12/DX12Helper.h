#pragma once

#include "Renderer/DX12/DX12Common.h"

namespace Xunlan::Graphics::DX12::Helper
{
    namespace Heap
    {
        constexpr D3D12_HEAP_PROPERTIES DEFAULT_HEAP = {
            D3D12_HEAP_TYPE_DEFAULT,            // D3D12_HEAP_TYPE Type
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,    // D3D12_CPU_PAGE_PROPERTY CPUPageProperty
            D3D12_MEMORY_POOL_UNKNOWN,          // D3D12_MEMORY_POOL MemoryPoolPreference
            0,                                  // UINT CreationNodeMask
            0,                                  // UINT VisibleNodeMask
        };

        constexpr D3D12_HEAP_PROPERTIES UPLOAD_HEAP = {
            D3D12_HEAP_TYPE_UPLOAD,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            0,
            0,
        };
    }

    namespace Resource
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(
            const void* data,
            uint64 bufferSize,
            bool isCPUAccessible = false,
            D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON,
            ID3D12Heap* heap = nullptr,
            uint64 heapOffset = 0
        );
    }

    namespace RootSig
    {
        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateRootSignature(Device& device, const D3D12_ROOT_SIGNATURE_DESC1& desc);

        class DescriptorRange : public D3D12_DESCRIPTOR_RANGE1
        {
        public:

            explicit DescriptorRange(
                D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
                uint32 numDescriptors,    // can be D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND, which means boundless range
                uint32 baseShaderRegister,
                uint32 registerSpace = 0,
                D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
                uint32 offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
                : D3D12_DESCRIPTOR_RANGE1{ rangeType, numDescriptors, baseShaderRegister, registerSpace, flags, offsetInDescriptorsFromTableStart } {}
        };

        class RootParameter : public D3D12_ROOT_PARAMETER1
        {
        public:

            void InitAsDescriptorTable(
                UINT numDescriptorRanges,
                const DescriptorRange* pDescriptorRanges,
                D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
            {
                ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                DescriptorTable.NumDescriptorRanges = numDescriptorRanges;
                DescriptorTable.pDescriptorRanges = pDescriptorRanges;
                ShaderVisibility = visibility;
            }

            void InitAsConstants(
                UINT num32BitValues,
                UINT shaderRegister,
                UINT registerSpace = 0,
                D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
            {
                ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                Constants.Num32BitValues = num32BitValues;
                Constants.ShaderRegister = shaderRegister;
                Constants.RegisterSpace = registerSpace;
                ShaderVisibility = visibility;
            }

            void InitAsCBV(
                UINT shaderRegister,
                UINT registerSpace = 0,
                D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
            {
                InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, shaderRegister, registerSpace, flags, visibility);
            }

            void InitAsSRV(
                UINT shaderRegister,
                UINT registerSpace = 0,
                D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
            {
                InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, shaderRegister, registerSpace, flags, visibility);
            }

            void InitAsUAV(
                UINT shaderRegister,
                UINT registerSpace = 0,
                D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
            {
                InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, shaderRegister, registerSpace, flags, visibility);
            }

        private:

            void InitAsDescriptor(
                D3D12_ROOT_PARAMETER_TYPE descriptorType,
                UINT shaderRegister,
                UINT registerSpace = 0,
                D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
            {
                ParameterType = descriptorType;
                Descriptor.ShaderRegister = shaderRegister;
                Descriptor.RegisterSpace = registerSpace;
                Descriptor.Flags = flags;
                ShaderVisibility = visibility;
            }
        };

        class RootSignatureDesc : public D3D12_ROOT_SIGNATURE_DESC1
        {
        public:

            explicit RootSignatureDesc(
                UINT numParameters,
                const D3D12_ROOT_PARAMETER1* pParameters,
                UINT numStaticSamplers = 0,
                const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = nullptr,
                D3D12_ROOT_SIGNATURE_FLAGS flags =
                D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS)
                : D3D12_ROOT_SIGNATURE_DESC1{ numParameters, pParameters, numStaticSamplers, pStaticSamplers, flags } {}
        };
    }

    namespace Rasterizer
    {
        constexpr D3D12_RASTERIZER_DESC NO_CULL = {
            D3D12_FILL_MODE_SOLID,                      // D3D12_FILL_MODE FillMode
            D3D12_CULL_MODE_NONE,                       // D3D12_CULL_MODE CullMode
            FALSE,                                      // BOOL FrontCounterClockwise
            0,                                          // INT DepthBias
            0.0f,                                       // FLOAT DepthBiasClamp
            0.0f,                                       // FLOAT SlopeScaledDepthBias
            TRUE,                                       // BOOL DepthClipEnable
            TRUE,                                       // BOOL MultisampleEnable
            FALSE,                                      // BOOL AntialiasedLineEnable
            0,                                          // UINT ForcedSampleCount
            D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,  // D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster
        };
        constexpr D3D12_RASTERIZER_DESC BACK_CULL = {
            D3D12_FILL_MODE_SOLID,                      // D3D12_FILL_MODE FillMode
            D3D12_CULL_MODE_BACK,                       // D3D12_CULL_MODE CullMode
            FALSE,                                      // BOOL FrontCounterClockwise
            0,                                          // INT DepthBias
            0.0f,                                       // FLOAT DepthBiasClamp
            0.0f,                                       // FLOAT SlopeScaledDepthBias
            TRUE,                                       // BOOL DepthClipEnable
            TRUE,                                       // BOOL MultisampleEnable
            FALSE,                                      // BOOL AntialiasedLineEnable
            0,                                          // UINT ForcedSampleCount
            D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,  // D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster
        };
        constexpr D3D12_RASTERIZER_DESC FRONT_CULL = {
            D3D12_FILL_MODE_SOLID,                      // D3D12_FILL_MODE FillMode
            D3D12_CULL_MODE_FRONT,                      // D3D12_CULL_MODE CullMode
            FALSE,                                      // BOOL FrontCounterClockwise
            0,                                          // INT DepthBias
            0.0f,                                       // FLOAT DepthBiasClamp
            0.0f,                                       // FLOAT SlopeScaledDepthBias
            TRUE,                                       // BOOL DepthClipEnable
            TRUE,                                       // BOOL MultisampleEnable
            FALSE,                                      // BOOL AntialiasedLineEnable
            0,                                          // UINT ForcedSampleCount
            D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,  // D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster
        };
        constexpr D3D12_RASTERIZER_DESC WIREFRAME = {
            D3D12_FILL_MODE_WIREFRAME,                  // D3D12_FILL_MODE FillMode
            D3D12_CULL_MODE_NONE,                       // D3D12_CULL_MODE CullMode
            FALSE,                                      // BOOL FrontCounterClockwise
            0,                                          // INT DepthBias
            0.0f,                                       // FLOAT DepthBiasClamp
            0.0f,                                       // FLOAT SlopeScaledDepthBias
            TRUE,                                       // BOOL DepthClipEnable
            TRUE,                                       // BOOL MultisampleEnable
            FALSE,                                      // BOOL AntialiasedLineEnable
            0,                                          // UINT ForcedSampleCount
            D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,  // D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster
        };
    }

    namespace DepthStencil
    {
        constexpr D3D12_DEPTH_STENCIL_DESC1 DISABLED = {
            FALSE,                              // BOOL DepthEnable
            D3D12_DEPTH_WRITE_MASK_ZERO,        // D3D12_DEPTH_WRITE_MASK DepthWriteMask
            D3D12_COMPARISON_FUNC_LESS_EQUAL,   // D3D12_COMPARISON_FUNC DepthFunc
            FALSE,                              // BOOL StencilEnable
            0,                                  // UINT8 StencilReadMask
            0,                                  // UINT8 StencilWriteMask
            {},                                 // D3D12_DEPTH_STENCILOP_DESC FrontFace
            {},                                 // D3D12_DEPTH_STENCILOP_DESC BackFace
            FALSE,                              // BOOL DepthBoundsTestEnable
        };
    }

    namespace PSO
    {
        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12PipelineState> CreatePipelineState(Device& device, void* stream, uint64 streamByteSize);

        template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type, typename T>
        class alignas(void*) Subobject
        {
        public:

            Subobject() = default;
            Subobject(const T& subobject) : m_subobject(subobject) {}
            Subobject& operator=(const Subobject& rhs)
            {
                if (this == &rhs) return *this;
                m_subobject = rhs.m_subobject;
                return *this;
            }

        private:

            const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE m_type = type;
            T m_subobject = {};
        };

        using Subobject_RootSignature = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*>;
        using Subobject_VS = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS, D3D12_SHADER_BYTECODE>;
        using Subobject_PS = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS, D3D12_SHADER_BYTECODE>;
        using Subobject_DS = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS, D3D12_SHADER_BYTECODE>;
        using Subobject_HS = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS, D3D12_SHADER_BYTECODE>;
        using Subobject_GS = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS, D3D12_SHADER_BYTECODE>;
        using Subobject_CS = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS, D3D12_SHADER_BYTECODE>;
        using Subobject_StreamOutput = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT, D3D12_STREAM_OUTPUT_DESC>;
        using Subobject_Blend = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND, D3D12_BLEND_DESC>;
        using Subobject_SampleMask = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK, uint32>;
        using Subobject_Rasterizer = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER, D3D12_RASTERIZER_DESC>;
        using Subobject_DepthStencil = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL, D3D12_DEPTH_STENCIL_DESC>;
        using Subobject_InputLayout = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT, D3D12_INPUT_LAYOUT_DESC>;
        using Subobject_IBStripCutValue = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE, D3D12_INDEX_BUFFER_STRIP_CUT_VALUE>;
        using Subobject_PrimitiveTopology = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY, D3D12_PRIMITIVE_TOPOLOGY_TYPE>;
        using Subobject_RenderTargetFormats = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS, D3D12_RT_FORMAT_ARRAY>;
        using Subobject_DepthStencilFormat = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT, DXGI_FORMAT>;
        using Subobject_SampleDesc = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC, DXGI_SAMPLE_DESC>;
        using Subobject_NodeMask = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK, uint32>;
        using Subobject_CachedPSO = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO, D3D12_CACHED_PIPELINE_STATE>;
        using Subobject_Flags = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS, D3D12_PIPELINE_STATE_FLAGS>;
        using Subobject_DepthStencil1 = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1, D3D12_DEPTH_STENCIL_DESC1>;
        using Subobject_ViewInstancing = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING, D3D12_VIEW_INSTANCING_DESC>;
        using Subobject_AS = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS, D3D12_SHADER_BYTECODE>;
        using Subobject_MS = Subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS, D3D12_SHADER_BYTECODE>;
    }

    namespace Barrier
    {
        class BarrierContainer final
        {
        public:

            BarrierContainer() = default;
            BarrierContainer(const BarrierContainer& rhs) = delete;
            BarrierContainer& operator=(const BarrierContainer& rhs) = delete;
            BarrierContainer(BarrierContainer&& rhs) = delete;
            BarrierContainer& operator=(BarrierContainer&& rhs) = delete;

        public:

            void AddTransition(
                ID3D12Resource& resource,
                D3D12_RESOURCE_STATES before,
                D3D12_RESOURCE_STATES after,
                D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                uint32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
            void AddAliasing(
                ID3D12Resource& resourceBefore,
                ID3D12Resource& resourceAfter,
                D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
            void AddUAV(
                ID3D12Resource& resource,
                D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

            void Commit(GraphicsCommandList& cmdList);

            static constexpr uint32 MAX_NUM_BARRIERS = 32;

        private:

            D3D12_RESOURCE_BARRIER m_barriers[MAX_NUM_BARRIERS] = {};
            uint32 m_size = 0;
        };

        void Transition(
            GraphicsCommandList& cmdList,
            ID3D12Resource& resource,
            D3D12_RESOURCE_STATES before,
            D3D12_RESOURCE_STATES after,
            uint32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

        void Aliasing(
            GraphicsCommandList& cmdList,
            ID3D12Resource& resourceBefore,
            ID3D12Resource& resourceAfter,
            D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

        void UAV(
            GraphicsCommandList& cmdList,
            ID3D12Resource& resource,
            D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
    }
}