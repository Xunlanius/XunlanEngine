#pragma once

#include "DX12Common.h"

namespace Xunlan::DX12::Helper
{
    namespace Resource
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> UploadToDefaultBuffer(GraphicsCommandList& cmdList, ID3D12Resource* defaultResource, const void* data, size_t byteSize);
    }

    namespace RootSig
    {
        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateRootSignature(Device& device, const D3D12_ROOT_SIGNATURE_DESC1& desc);
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
        constexpr D3D12_DEPTH_STENCIL_DESC1 ENABLED = {
            TRUE,                                   // BOOL DepthEnable
            D3D12_DEPTH_WRITE_MASK_ALL,             // D3D12_DEPTH_WRITE_MASK DepthWriteMask
            D3D12_COMPARISON_FUNC_LESS_EQUAL,       // D3D12_COMPARISON_FUNC DepthFunc
            FALSE,                                  // BOOL StencilEnable
            0,                                      // UINT8 StencilReadMask
            0,                                      // UINT8 StencilWriteMask
            {},                                     // D3D12_DEPTH_STENCILOP_DESC FrontFace
            {},                                     // D3D12_DEPTH_STENCILOP_DESC BackFace
            FALSE,                                  // BOOL DepthBoundsTestEnable
        };

        constexpr D3D12_DEPTH_STENCIL_DESC1 ENABLED_READONLY = {
            TRUE,                                   // BOOL DepthEnable
            D3D12_DEPTH_WRITE_MASK_ZERO,            // D3D12_DEPTH_WRITE_MASK DepthWriteMask
            D3D12_COMPARISON_FUNC_EQUAL,            // D3D12_COMPARISON_FUNC DepthFunc
            FALSE,                                  // BOOL StencilEnable
            0,                                      // UINT8 StencilReadMask
            0,                                      // UINT8 StencilWriteMask
            {},                                     // D3D12_DEPTH_STENCILOP_DESC FrontFace
            {},                                     // D3D12_DEPTH_STENCILOP_DESC BackFace
            FALSE,                                  // BOOL DepthBoundsTestEnable
        };

        constexpr D3D12_DEPTH_STENCIL_DESC1 DISABLED = {
            FALSE,                                  // BOOL DepthEnable
            D3D12_DEPTH_WRITE_MASK_ZERO,            // D3D12_DEPTH_WRITE_MASK DepthWriteMask
            D3D12_COMPARISON_FUNC_LESS_EQUAL,       // D3D12_COMPARISON_FUNC DepthFunc
            FALSE,                                  // BOOL StencilEnable
            0,                                      // UINT8 StencilReadMask
            0,                                      // UINT8 StencilWriteMask
            {},                                     // D3D12_DEPTH_STENCILOP_DESC FrontFace
            {},                                     // D3D12_DEPTH_STENCILOP_DESC BackFace
            FALSE,                                  // BOOL DepthBoundsTestEnable
        };

        constexpr D3D12_DEPTH_STENCIL_DESC1 REVERSE = {
            TRUE,                                   // BOOL DepthEnable
            D3D12_DEPTH_WRITE_MASK_ALL,             // D3D12_DEPTH_WRITE_MASK DepthWriteMask
            D3D12_COMPARISON_FUNC_GREATER_EQUAL,    // D3D12_COMPARISON_FUNC DepthFunc
            FALSE,                                  // BOOL StencilEnable
            0,                                      // UINT8 StencilReadMask
            0,                                      // UINT8 StencilWriteMask
            {},                                     // D3D12_DEPTH_STENCILOP_DESC FrontFace
            {},                                     // D3D12_DEPTH_STENCILOP_DESC BackFace
            FALSE,                                  // BOOL DepthBoundsTestEnable
        };

        constexpr D3D12_DEPTH_STENCIL_DESC1 REVERSE_READONLY = {
            TRUE,                                   // BOOL DepthEnable
            D3D12_DEPTH_WRITE_MASK_ZERO,            // D3D12_DEPTH_WRITE_MASK DepthWriteMask
            D3D12_COMPARISON_FUNC_EQUAL,            // D3D12_COMPARISON_FUNC DepthFunc
            FALSE,                                  // BOOL StencilEnable
            0,                                      // UINT8 StencilReadMask
            0,                                      // UINT8 StencilWriteMask
            {},                                     // D3D12_DEPTH_STENCILOP_DESC FrontFace
            {},                                     // D3D12_DEPTH_STENCILOP_DESC BackFace
            FALSE,                                  // BOOL DepthBoundsTestEnable
        };
    }

    namespace Blend
    {
        constexpr D3D12_BLEND_DESC DISABLED = {
            FALSE,                                  // BOOL AlphaToCoverageEnable
            FALSE,                                  // BOOL IndependentBlendEnable
            {
                {
                    FALSE,                          // BOOL BlendEnable
                    FALSE,                          // BOOL LogicOpEnable
                    D3D12_BLEND_SRC_ALPHA,          // D3D12_BLEND SrcBlend
                    D3D12_BLEND_INV_SRC_ALPHA,      // D3D12_BLEND DestBlend
                    D3D12_BLEND_OP_ADD,             // D3D12_BLEND_OP BlendOp
                    D3D12_BLEND_ONE,                // D3D12_BLEND SrcBlendAlpha
                    D3D12_BLEND_ONE,                // D3D12_BLEND DestBlendAlpha
                    D3D12_BLEND_OP_ADD,             // D3D12_BLEND_OP BlendOpAlpha
                    D3D12_LOGIC_OP_NOOP,            // D3D12_LOGIC_OP LogicOp
                    D3D12_COLOR_WRITE_ENABLE_ALL,   // UINT8 RenderTargetWriteMask
                }, {}, {}, {}, {}, {}, {}, {},
            }                                       // D3D12_RENDER_TARGET_BLEND_DESC RenderTarget[8]
        };

        constexpr D3D12_BLEND_DESC ALPHA_BLEND = {
            FALSE,                                  // BOOL AlphaToCoverageEnable
            FALSE,                                  // BOOL IndependentBlendEnable
            {
                {
                    TRUE,                           // BOOL BlendEnable
                    FALSE,                          // BOOL LogicOpEnable
                    D3D12_BLEND_SRC_ALPHA,          // D3D12_BLEND SrcBlend
                    D3D12_BLEND_INV_SRC_ALPHA,      // D3D12_BLEND DestBlend
                    D3D12_BLEND_OP_ADD,             // D3D12_BLEND_OP BlendOp
                    D3D12_BLEND_ONE,                // D3D12_BLEND SrcBlendAlpha
                    D3D12_BLEND_ONE,                // D3D12_BLEND DestBlendAlpha
                    D3D12_BLEND_OP_ADD,             // D3D12_BLEND_OP BlendOpAlpha
                    D3D12_LOGIC_OP_NOOP,            // D3D12_LOGIC_OP LogicOp
                    D3D12_COLOR_WRITE_ENABLE_ALL,   // UINT8 RenderTargetWriteMask
                }, {}, {}, {}, {}, {}, {}, {},
            }                                       // D3D12_RENDER_TARGET_BLEND_DESC RenderTarget[8]
        };

        constexpr D3D12_BLEND_DESC ADDITIVE = {
            FALSE,                                  // BOOL AlphaToCoverageEnable
            FALSE,                                  // BOOL IndependentBlendEnable
            {
                {
                    TRUE,                           // BOOL BlendEnable
                    FALSE,                          // BOOL LogicOpEnable
                    D3D12_BLEND_ONE,                // D3D12_BLEND SrcBlend
                    D3D12_BLEND_ONE,                // D3D12_BLEND DestBlend
                    D3D12_BLEND_OP_ADD,             // D3D12_BLEND_OP BlendOp
                    D3D12_BLEND_ONE,                // D3D12_BLEND SrcBlendAlpha
                    D3D12_BLEND_ONE,                // D3D12_BLEND DestBlendAlpha
                    D3D12_BLEND_OP_ADD,             // D3D12_BLEND_OP BlendOpAlpha
                    D3D12_LOGIC_OP_NOOP,            // D3D12_LOGIC_OP LogicOp
                    D3D12_COLOR_WRITE_ENABLE_ALL,   // UINT8 RenderTargetWriteMask
                }, {}, {}, {}, {}, {}, {}, {},
            }                                       // D3D12_RENDER_TARGET_BLEND_DESC RenderTarget[8]
        };

        constexpr D3D12_BLEND_DESC PREMULTIPLIED = {
            FALSE,                                  // BOOL AlphaToCoverageEnable
            FALSE,                                  // BOOL IndependentBlendEnable
            {
                {
                    FALSE,                          // BOOL BlendEnable
                    FALSE,                          // BOOL LogicOpEnable
                    D3D12_BLEND_ONE,                // D3D12_BLEND SrcBlend
                    D3D12_BLEND_INV_SRC_ALPHA,      // D3D12_BLEND DestBlend
                    D3D12_BLEND_OP_ADD,             // D3D12_BLEND_OP BlendOp
                    D3D12_BLEND_ONE,                // D3D12_BLEND SrcBlendAlpha
                    D3D12_BLEND_ONE,                // D3D12_BLEND DestBlendAlpha
                    D3D12_BLEND_OP_ADD,             // D3D12_BLEND_OP BlendOpAlpha
                    D3D12_LOGIC_OP_NOOP,            // D3D12_LOGIC_OP LogicOp
                    D3D12_COLOR_WRITE_ENABLE_ALL,   // UINT8 RenderTargetWriteMask
                }, {}, {}, {}, {}, {}, {}, {},
            }                                       // D3D12_RENDER_TARGET_BLEND_DESC RenderTarget[8]
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

        struct DX12PipelineStateSubobjectStream
        {
            Subobject_RootSignature rootSignature = nullptr;
            Subobject_VS vs = {};
            Subobject_PS ps = {};
            Subobject_DS ds = {};
            Subobject_HS hs = {};
            Subobject_GS gs = {};
            Subobject_CS cs = {};
            Subobject_StreamOutput streamOutput = {};
            Subobject_Blend blend = Blend::DISABLED;
            Subobject_SampleMask sampleMask = { UINT32_MAX };
            Subobject_Rasterizer rasterizer = {};
            Subobject_InputLayout inputLayout = {};
            Subobject_IBStripCutValue ibStripCutValue = {};
            Subobject_PrimitiveTopology primitiveTopology = {};
            Subobject_RenderTargetFormats renderTargetFormats = {};
            Subobject_DepthStencilFormat depthStencilFormat = {};
            Subobject_SampleDesc sampleDesc = {{ 1, 0 }};
            Subobject_NodeMask nodeMask = {};
            Subobject_CachedPSO cachedPSO = {};
            Subobject_Flags flags = {};
            Subobject_DepthStencil1 depthStencil1 = DepthStencil::DISABLED;
            Subobject_ViewInstancing viewInstancing = {};
            Subobject_AS as = {};
            Subobject_MS ms = {};
        };
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