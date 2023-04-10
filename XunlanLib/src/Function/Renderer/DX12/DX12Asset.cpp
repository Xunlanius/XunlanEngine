#include "DX12Asset.h"
#include "DX12RootParameter.h"
#include "DX12Helper.h"
#include "DX12Resource.h"
#include "src/Function/Core/RuntimeContext.h"
#include "src/Function/Resource/AssetSystem.h"
#include "src/Function/Renderer/RendererSystem.h"
#include "src/Utility/IO.h"
#include "src/Utility/MathTypes.h"
#include "src/Utility/Math.h"
#include "src/Utility/Allocator.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <mutex>

#include <dxcapi.h>
#include <d3d12shader.h>

#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;

namespace Xunlan::DX12::Asset
{
    namespace Submesh
    {
        namespace
        {
            Utility::FreeList<DX12Submesh> g_submeshes;
            std::mutex g_mutex = {};

            D3D_PRIMITIVE_TOPOLOGY GetD3DPrimitiveTopology(PrimitiveTopology primitiveTopology)
            {
                assert(primitiveTopology < PrimitiveTopology::Count);

                switch (primitiveTopology)
                {
                    case PrimitiveTopology::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                    case PrimitiveTopology::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                    case PrimitiveTopology::LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                    case PrimitiveTopology::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                    case PrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                    default: return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
                }
            }
            D3D12_PRIMITIVE_TOPOLOGY_TYPE GetDX12PrimitiveTopologyType(D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
            {
                switch (primitiveTopology)
                {
                    case D3D_PRIMITIVE_TOPOLOGY_POINTLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
                    case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
                    case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
                    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
                    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                    default: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
                }
            }
        }

        LongID UploadSubmesh(const SubmeshData& data)
        {
            const uint32 numVertices = (uint32)data.vertexBuffer.size();
            const uint32 numIndices = (uint32)data.indexBuffer.size();

            const uint32 vertexStride = sizeof(decltype(data.vertexBuffer)::value_type);
            const uint32 indexStride = sizeof(decltype(data.indexBuffer)::value_type);

            // 4-bytes aligned
            const uint32 vertexBufferSize = numVertices * vertexStride;
            const uint32 indexBufferSize = numIndices * indexStride;
            const uint32 totalBufferSize = vertexBufferSize + indexBufferSize;

            std::vector<byte> buffer(totalBufferSize);
            byte* dst = buffer.data();
            IO::WriteBuffer(dst, data.vertexBuffer.data(), vertexBufferSize);
            IO::WriteBuffer(dst, data.indexBuffer.data(), indexBufferSize);
            assert(dst == &buffer.back() + 1);

            // Position and Element buffer should be aligned to 4-bytes
            constexpr uint32 align = D3D12_STANDARD_MAXIMUM_ELEMENT_ALIGNMENT_BYTE_MULTIPLE;

            DX12Submesh dx12Submesh = {};

            dx12Submesh.buffer = Helper::Resource::CreateBuffer(buffer.data(), totalBufferSize);
            NAME_OBJECT_INDEX(dx12Submesh.buffer, L"Submesh Buffer - size: ", totalBufferSize);

            D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = dx12Submesh.vertexBufferView;
            D3D12_INDEX_BUFFER_VIEW& indexBufferView = dx12Submesh.indexBufferView;

            vertexBufferView.BufferLocation = dx12Submesh.buffer->GetGPUVirtualAddress();
            vertexBufferView.SizeInBytes = vertexBufferSize;
            vertexBufferView.StrideInBytes = vertexStride;

            indexBufferView.BufferLocation = vertexBufferView.BufferLocation + vertexBufferSize;
            indexBufferView.SizeInBytes = indexBufferSize;
            indexBufferView.Format = DXGI_FORMAT_R32_UINT;

            dx12Submesh.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

            std::lock_guard lock(g_mutex);
            return g_submeshes.Emplace(dx12Submesh);
        }
        void UnloadSubmesh(LongID submeshID)
        {
            std::lock_guard lock(g_mutex);

            g_dx12RHI->DeferredRelease(g_submeshes[submeshID].buffer);
            g_submeshes.Remove(submeshID);
        }

        const DX12Submesh& GetSubmesh(LongID submeshID) { return g_submeshes[submeshID]; }
    }

    namespace Texture
    {
        namespace
        {
            Utility::FreeList<DX12Texture> g_textures;
            std::mutex g_mutex;
        }

        LongID UploadTexture(const byte* data)
        {
            return LongID();
        }
        void UnloadTexture(LongID textureID)
        {

        }

        void GetDescriptorIndices(std::vector<DX12Material::TextureInfo>& textureInfos)
        {
            std::lock_guard lock(g_mutex);

            for (auto& info : textureInfos)
            {
                info.descriptorIndex = g_textures[info.textureID].GetSRV().index;
            }
        }
    }

    namespace Material
    {
        namespace
        {
            std::unordered_map<uint64, ComPtr<ID3D12RootSignature>> g_materialToRootSig;
            std::vector<ComPtr<ID3D12RootSignature>> g_rootSignatures;

            Utility::FreeList<DX12Material> g_materials;
            std::mutex g_mutex;

            D3D12_ROOT_SIGNATURE_FLAGS GetFlags(ShaderFlags::Value flags)
            {
                D3D12_ROOT_SIGNATURE_FLAGS result = Helper::RootSig::RootSignatureDesc::ms_defaultFlags;
                if (flags & ShaderFlags::Vertex) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
                if (flags & ShaderFlags::Hull) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
                if (flags & ShaderFlags::Domain) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
                if (flags & ShaderFlags::Geometry) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
                if (flags & ShaderFlags::Pixel) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
                if (flags & ShaderFlags::Amplification) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
                if (flags & ShaderFlags::Mesh) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

                return result;
            }

            ComPtr<ID3D12RootSignature> UploadRootSignature(MaterialType type, ShaderFlags::Value flags)
            {
                assert(type < MaterialType::Count);

                const uint64 key = ((uint64)type << 32) | (uint64)flags;

                // If the key is existed, return
                auto it = g_materialToRootSig.find(key);
                if (it != g_materialToRootSig.end()) return it->second;

                // Create a new root signature
                ComPtr<ID3D12RootSignature> rootSig = nullptr;

                switch (type)
                {
                    case MaterialType::Opaque:
                    {
                        Helper::RootSig::RootParameter params[(uint32)OpaqueRootParam::Count] = {};

                        D3D12_SHADER_VISIBILITY bufferVisibility = {};
                        D3D12_SHADER_VISIBILITY dataVisibility = {};

                        if (flags & ShaderFlags::Vertex)
                        {
                            bufferVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
                            dataVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
                        }
                        else if (flags & ShaderFlags::Mesh)
                        {
                            bufferVisibility = D3D12_SHADER_VISIBILITY_MESH;
                            dataVisibility = D3D12_SHADER_VISIBILITY_MESH;
                        }

                        if ((flags & ShaderFlags::Hull) || (flags & ShaderFlags::Geometry) || (flags & ShaderFlags::Amplification))
                        {
                            bufferVisibility = D3D12_SHADER_VISIBILITY_ALL;
                            dataVisibility = D3D12_SHADER_VISIBILITY_ALL;
                        }

                        if ((flags & ShaderFlags::Pixel) || (flags & ShaderFlags::Compute))
                        {
                            dataVisibility = D3D12_SHADER_VISIBILITY_ALL;
                        }

                        params[(uint32)OpaqueRootParam::PER_FRAME].InitAsCBV(0, D3D12_SHADER_VISIBILITY_ALL);
                        params[(uint32)OpaqueRootParam::PER_OBJECT].InitAsCBV(1, dataVisibility);
                        params[(uint32)OpaqueRootParam::VERTEX_BUFFER].InitAsSRV(0, bufferVisibility);
                        params[(uint32)OpaqueRootParam::SRVIndices].InitAsSRV(1, D3D12_SHADER_VISIBILITY_PIXEL);

                        Helper::RootSig::RootSignatureDesc desc(_countof(params), params, 0, nullptr, GetFlags(flags));
                        rootSig = Helper::RootSig::CreateRootSignature(g_dx12RHI->GetDevice(), desc);

                        break;
                    }
                }

                assert(rootSig);
                NAME_OBJECT_INDEX(rootSig, L"Root Signature - key: ", key);

                g_materialToRootSig.emplace(key, rootSig);
                g_rootSignatures.emplace_back(rootSig);
                return rootSig;
            }
        }

        void Shutdown()
        {
            g_materialToRootSig.clear();
            g_rootSignatures.clear();
        }

        LongID UploadMaterial(const MaterialInitDesc& initDesc)
        {
            std::lock_guard lock(g_mutex);

            const LongID materialID = g_materials.Emplace();
            DX12Material& material = g_materials[materialID];

            material.type = initDesc.type;
            memcpy(material.shaderIDs, initDesc.shaderIDs, sizeof(material.shaderIDs));

            size_t numTextures = initDesc.textureIDs.size();
            material.textures.resize(numTextures);

            uint32 flags = 0;
            for (uint32 i = 0; i < (uint32)ShaderType::Count; ++i)
            {
                if (material.shaderIDs[i] != INVALID_LONG_ID) flags |= (ShaderFlags::Value)(1 << i);
            }
            material.flags = (ShaderFlags::Value)flags;
            material.rootSignature = UploadRootSignature(material.type, (ShaderFlags::Value)flags);

            for (uint32 i = 0; i < numTextures; ++i)
            {
                material.textures[i].textureID = initDesc.textureIDs[i];
            }
            Texture::GetDescriptorIndices(material.textures);

            return materialID;
        }
        void UnloadMaterial(LongID materialID)
        {
            std::lock_guard lock(g_mutex);
            g_materials.Remove(materialID);
        }

        const DX12Material& GetMaterial(LongID materialID)
        {
            std::lock_guard lock(g_mutex);
            return g_materials[materialID];
        }
    }

    namespace RenderItem
    {
        namespace
        {
            Utility::FreeList<DX12RenderItem> g_renderItems;

            std::unordered_map<uint64, ComPtr<ID3D12PipelineState>> g_psoMap;
            std::vector<ComPtr<ID3D12PipelineState>> g_PSOs;

            std::mutex g_mutex;

            ID3D12PipelineState* CreatePSOIfNeeded(Helper::PSO::DX12PipelineStateSubobjectStream& stream)
            {
                const uint64 key = Math::ComputeCRC32Uint64((byte*)&stream, Math::RoundUp<sizeof(uint64)>(sizeof(stream)));

                auto it = g_psoMap.find(key);
                if (it != g_psoMap.end()) return it->second.Get();

                ComPtr<ID3D12PipelineState> pso = Helper::PSO::CreatePipelineState(g_dx12RHI->GetDevice(), &stream, sizeof(stream));
                NAME_OBJECT_INDEX(pso, L"PSO - key: ", key);

                g_psoMap.emplace(key, pso);
                g_PSOs.emplace_back(pso);

                return pso.Get();
            }

            std::vector<ID3D12PipelineState*> CreatePSO(LongID materialID, D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
            {
                D3D12_RT_FORMAT_ARRAY rtArray = {};
                rtArray.NumRenderTargets = 1;
                rtArray.RTFormats[0] = MAIN_GBUFFER_FORMAT;

                const DX12Material& material = Material::GetMaterial(materialID);

                Helper::PSO::DX12PipelineStateSubobjectStream stream = {};
                stream.rootSignature = material.rootSignature.Get();
                stream.blend = Helper::Blend::DISABLED;
                stream.rasterizer = Helper::Rasterizer::BACK_CULL;
                stream.primitiveTopology = Submesh::GetDX12PrimitiveTopologyType(primitiveTopology);
                stream.renderTargetFormats = rtArray;
                stream.depthStencilFormat = DEPTH_GBUFFER_FORMAT;
                stream.depthStencil1 = Helper::DepthStencil::ENABLED;

                const ShaderFlags::Value shaderFlags = material.flags;
                D3D12_SHADER_BYTECODE shaders[(uint32)ShaderType::Count] = {};

                for (uint32 i = 0; i < (uint32)ShaderType::Count; ++i)
                {
                    if (shaderFlags & (1u << i))
                    {
                        const LongID shaderID = material.shaderIDs[i];
                        const CompiledShader& shader = g_runtimeContext.m_assetSystem->GetShader(shaderID);

                        shaders[i].BytecodeLength = shader.byteCode.size();
                        shaders[i].pShaderBytecode = shader.byteCode.data();
                    }
                }

                stream.vs = shaders[(uint32)ShaderType::Vertex];
                stream.ps = {};
                stream.ds = shaders[(uint32)ShaderType::Domain];
                stream.hs = shaders[(uint32)ShaderType::Hull];
                stream.gs = shaders[(uint32)ShaderType::Geometry];
                stream.cs = shaders[(uint32)ShaderType::Compute];
                stream.as = shaders[(uint32)ShaderType::Amplification];
                stream.ms = shaders[(uint32)ShaderType::Mesh];

                std::vector<ID3D12PipelineState*> result;
                result.emplace_back(CreatePSOIfNeeded(stream));

                stream.ps = shaders[(uint32)ShaderType::Pixel];
                stream.depthStencil1 = Helper::DepthStencil::ENABLED_READONLY;

                result.emplace_back(CreatePSOIfNeeded(stream));

                return result;
            }
        }

        void Shutdown()
        {
            g_psoMap.clear();
            g_PSOs.clear();
        }

        LongID UploadRenderItem(ECS::EntityID entity, LongID submeshID, LongID materialID)
        {
            assert(entity != ECS::INVALID_ID && IsValid(submeshID) && IsValid(materialID));
            std::lock_guard lock(g_mutex);

            const DX12Submesh& dx12Submesh = Submesh::g_submeshes[submeshID];

            DX12RenderItem item = {};
            item.entity = entity;
            item.submeshID = submeshID;
            item.materialID = materialID;
            item.PSOs = CreatePSO(materialID, dx12Submesh.primitiveTopology);

            return g_renderItems.Emplace(item);
        }
        void UnloadRenderItem(LongID renderItemID)
        {
            std::lock_guard lock(g_mutex);
            g_renderItems.Remove(renderItemID);
        }

        std::vector<DX12RenderItem> GetRenderItems(const RenderResource& renderResource)
        {
            assert(renderResource.m_renderItemIDs.size() > 0);

            const uint32 numItems = (uint32)renderResource.m_renderItemIDs.size();
            std::vector<DX12RenderItem> result;
            result.reserve(numItems);

            std::lock_guard lock(g_mutex);

            for (const LongID itemID : renderResource.m_renderItemIDs)
            {
                result.push_back(g_renderItems[itemID]);
            }

            return result;
        }
    }

    bool Initialize(DX12RHI* rhi) { return true; }
    void Shutdown()
    {
        RenderItem::Shutdown();
        Material::Shutdown();
    }
}