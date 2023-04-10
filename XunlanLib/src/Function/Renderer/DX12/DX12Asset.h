#pragma once

#include "src/Platform/DX12/DX12Common.h"
#include "DX12RHI.h"
#include "DX12AssetCommon.h"
#include "src/Function/World/ECS/ECS.h"
#include "src/Function/Resource/AssetCommon.h"
#include "src/Function/Renderer/RendererSystem.h"

namespace Xunlan::DX12::Asset
{
    bool Initialize(DX12RHI* rhi);
    void Shutdown();

    namespace Submesh
    {
        LongID UploadSubmesh(const SubmeshData& data);
        void UnloadSubmesh(LongID submeshID);

        const DX12Submesh& GetSubmesh(LongID submeshID);
    }

    namespace Texture
    {
        LongID UploadTexture(const byte* data);
        void UnloadTexture(LongID textureID);

        void GetDescriptorIndices(std::vector<DX12Material::TextureInfo>& textureInfos);
    }

    namespace Material
    {
        bool Initialize();
        void Shutdown();

        LongID UploadMaterial(const MaterialInitDesc& initDesc);
        void UnloadMaterial(LongID materialID);

        const DX12Material& GetMaterial(LongID materialID);
    }

    namespace RenderItem
    {
        LongID UploadRenderItem(ECS::EntityID entity, LongID submeshID, LongID materialID);
        void UnloadRenderItem(LongID renderItemID);

        std::vector<DX12RenderItem> GetRenderItems(const RenderResource& renderResource);
    }
}