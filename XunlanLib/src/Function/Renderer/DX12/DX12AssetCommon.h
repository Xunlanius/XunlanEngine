#pragma once

#include "src/Platform/DX12/DX12Common.h"
#include "src/Function/Resource/AssetCommon.h"

namespace Xunlan::DX12::Asset
{
    struct DX12Submesh final
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> buffer;

        D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
        D3D12_INDEX_BUFFER_VIEW indexBufferView;

        D3D_PRIMITIVE_TOPOLOGY primitiveTopology;
    };

    struct DX12Material final
    {
        MaterialType type;
        ShaderFlags::Value flags;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        LongID shaderIDs[(uint32)ShaderType::Count];

        struct TextureInfo final
        {
            LongID textureID;
            uint32 descriptorIndex;
        };

        std::vector<TextureInfo> textures;
    };

    struct DX12RenderItem final
    {
        ECS::EntityID entity;
        LongID submeshID;
        LongID materialID;

        std::vector<ID3D12PipelineState*> PSOs;
    };
}