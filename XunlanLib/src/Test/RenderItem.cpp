#include "src/Common/Common.h"
#include "src/Function/Core/RuntimeContext.h"
#include "src/Function/World/Component/Transformer.h"
#include "src/Function/Renderer/RHI.h"
#include "src/Function/Renderer/RenderResource.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Function/Resource/AssetSystem.h"
#include "src/Tool/Shader/ShaderCompileSystem.h"

#include <thread>
#include <cassert>

namespace Xunlan
{
    namespace
    {
        ECS::EntityID g_entity;
        LongID g_modelID = INVALID_LONG_ID;

        LongID g_vsID = INVALID_LONG_ID;
        LongID g_psID = INVALID_LONG_ID;

        LongID g_materialID = INVALID_LONG_ID;

        void LoadModel()
        {
            std::filesystem::path modelPath = g_runtimeContext.m_configSystem->GetModelFolder();
            modelPath += "TestModel.model";

            MeshData meshData = {};
            g_runtimeContext.m_assetSystem->LoadAsset(modelPath, meshData);

            g_modelID = g_runtimeContext.m_assetSystem->UploadMesh(meshData);

            const TransformerInitDesc transformInitDesc = {
                { 0.0f, -2.0f, 0.0f },
                { -1.57f, 3.14f, 0.0f },
                { 1.0f, 1.0f, 1.0f },
            };

            const TransformerComponent transformer = CreateTransformer(transformInitDesc);

            ECS::ECSManager& instance = Singleton<ECS::ECSManager>::Instance();
            g_entity = instance.CreateEntity();
            instance.AddComponent(g_entity, transformer);
        }

        bool LoadShaders()
        {
            const std::filesystem::path& shaderFolder = g_runtimeContext.m_configSystem->GetHLSLShaderFolder();

            ShaderFileInfo vertexInfo = { "TestShader.hlsl", "TestShaderVS", ShaderType::Vertex };
            ShaderFileInfo pixelInfo = { "TestShader.hlsl", "TestShaderPS", ShaderType::Pixel };

            const CompiledShader vertexShader = g_runtimeContext.m_shaderCompileSystem->CompileShader(shaderFolder, vertexInfo);
            const CompiledShader pixelShader = g_runtimeContext.m_shaderCompileSystem->CompileShader(shaderFolder, pixelInfo);

            g_vsID = g_runtimeContext.m_assetSystem->UploadShader(vertexShader);
            g_psID = g_runtimeContext.m_assetSystem->UploadShader(pixelShader);

            return true;
        }

        LongID CreateMaterial()
        {
            MaterialInitDesc initDesc = {};
            initDesc.type = MaterialType::Opaque;
            initDesc.shaderIDs[(uint32)ShaderType::Vertex] = g_vsID;
            initDesc.shaderIDs[(uint32)ShaderType::Pixel] = g_psID;

            return g_runtimeContext.m_assetSystem->UploadMaterial(initDesc);
        }
    }

    void CreateRenderItem(RenderResource& renderResource)
    {
        std::thread _1 = std::thread(LoadModel);
        std::thread _2 = std::thread(LoadShaders);

        _1.join();
        _2.join();

        if (!IsValid(g_vsID) || !IsValid(g_psID)) return;

        g_materialID = CreateMaterial();

        const Mesh& mesh = g_runtimeContext.m_assetSystem->GetMesh(g_modelID);
        const uint32 numSubmeshes = mesh.GetNumSubmeshes();

        RenderEntity renderEntity;
        renderEntity.m_entity = g_entity;
        renderEntity.m_items.reserve(numSubmeshes);
        for (uint32 i = 0; i < numSubmeshes; ++i)
        {
            renderEntity.m_items.emplace_back(mesh.submeshIDs[i], g_materialID);
        }

        renderResource.UploadRenderItem(renderEntity);
    }
    void RemoveRenderItem()
    {
        g_runtimeContext.m_assetSystem->UnloadMaterial(g_materialID);

        g_runtimeContext.m_assetSystem->UnloadShader(g_vsID);
        g_runtimeContext.m_assetSystem->UnloadShader(g_psID);

        g_runtimeContext.m_assetSystem->UnloadMesh(g_modelID);
    }
}