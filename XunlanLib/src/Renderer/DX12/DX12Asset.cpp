#include "DX12Asset.h"
#include "Renderer/DX12/DX12Core.h"
#include "Utility/IO.h"
#include "Utility/MathTypes.h"
#include "Utility/Pool.h"
#include "Content/ContentToEngine.h"

using namespace Microsoft::WRL;

namespace Xunlan::Graphics::DX12::Asset
{
    namespace
    {
        struct PositionView
        {
            D3D12_VERTEX_BUFFER_VIEW positionBufferView = {};
            D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
        };

        struct ElementView
        {
            uint32 elementType = 0;
            D3D_PRIMITIVE_TOPOLOGY primitiveTopology = {};
            D3D12_VERTEX_BUFFER_VIEW elementBufferView = {};
        };

        struct SubmeshView
        {
            ComPtr<ID3D12Resource> buffer = nullptr;
            PositionView positionView = {};
            ElementView elementView = {};
        };

        Utility::FreeList<SubmeshView> g_submeshes;
        std::mutex g_mutex = {};

        D3D_PRIMITIVE_TOPOLOGY GetD3DPrimitiveTopology(Content::PrimitiveTopology primitiveTopology)
        {
            assert(primitiveTopology < Content::PrimitiveTopology::COUNT);

            switch (primitiveTopology)
            {
            case Content::PrimitiveTopology::POINT_LIST: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case Content::PrimitiveTopology::LINE_LIST: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case Content::PrimitiveTopology::LINE_STRIP: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case Content::PrimitiveTopology::TRIANGLE_LIST: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case Content::PrimitiveTopology::TRIANGLE_STRIP: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            default: return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }
    }

    namespace Submesh
    {
        ID Add(const byte*& data)
        {
            const uint32 numVertices = Utility::Read<uint32>(data);
            const uint32 numIndices = Utility::Read<uint32>(data);
            const uint32 elementType = Utility::Read<uint32>(data);
            const uint32 elementSize = Utility::Read<uint32>(data);
            const uint32 primitiveTopology = Utility::Read<uint32>(data);

            // 4-bytes aligned
            const uint32 positionBufferSize = numVertices * sizeof(Math::Vector3);
            const uint32 indexBufferSize = numIndices * sizeof(uint32);
            const uint32 elementBufferSize = numVertices * elementSize;
            const uint32 totalBufferSize = positionBufferSize + indexBufferSize + elementBufferSize;

            // Position and Element buffer should be aligned to 4-bytes
            constexpr uint32 align = D3D12_STANDARD_MAXIMUM_ELEMENT_ALIGNMENT_BYTE_MULTIPLE;

            SubmeshView submeshView = {};

            submeshView.buffer = Helper::Resource::CreateBuffer(data, totalBufferSize);
            data += totalBufferSize;

            PositionView& positionView = submeshView.positionView;
            D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = positionView.positionBufferView;
            D3D12_INDEX_BUFFER_VIEW& indexBufferView = positionView.indexBufferView;
            vertexBufferView.BufferLocation = submeshView.buffer->GetGPUVirtualAddress();
            vertexBufferView.SizeInBytes = positionBufferSize;
            vertexBufferView.StrideInBytes = sizeof(Math::Vector3);
            indexBufferView.BufferLocation = vertexBufferView.BufferLocation + positionBufferSize;
            indexBufferView.SizeInBytes = indexBufferSize;
            indexBufferView.Format = DXGI_FORMAT_R32_UINT;

            ElementView& elementView = submeshView.elementView;
            elementView.elementType = elementType;
            elementView.primitiveTopology = GetD3DPrimitiveTopology((Content::PrimitiveTopology)primitiveTopology);
            if (elementSize > 0)
            {
                elementView.elementBufferView.BufferLocation = indexBufferView.BufferLocation + indexBufferSize;
                elementView.elementBufferView.SizeInBytes = elementBufferSize;
                elementView.elementBufferView.StrideInBytes = elementSize;
            }

            std::lock_guard lock(g_mutex);
            return g_submeshes.Emplace(submeshView);
        }
        void Remove(ID id)
        {
            std::lock_guard lock(g_mutex);

            Core::DeferredRelease(g_submeshes[id].buffer);
            g_submeshes.Remove(id);
        }
    }
}