#include "PostProcessPass.h"
#include "src/Function/Renderer/RHI.h"
#include "src/Function/Resource/ConfigSystem.h"
#include "src/Function/Resource/ResourceType/MeshData.h"

namespace Xunlan
{
    PostProcessPass::PostProcessPass()
    {
        CreateMaterials();
        CreateCanvas();
    }

    void PostProcessPass::Render(const Ref<RenderContext>& context, PostProcessEffect effect, const Ref<RenderTarget>& inputRT)
    {
        RHI& rhi = RHI::Instance();

        rhi.SetRenderTarget(context, nullptr);
        rhi.ClearRenderTarget(context, nullptr);
        rhi.SetViewport(context, 0, 0, 0, 0);

        switch (effect)
        {
        case PostProcessEffect::NONE:
        {
            m_noneEffect->SetTexture(TextureCategory::BASE_COLOR, inputRT);
            m_canvas->Render(context, m_noneEffect);
            break;
        }
        default: assert(false);
        }

        rhi.ResetRenderTarget(context, nullptr);
    }

    void PostProcessPass::CreateMaterials()
    {
        RHI& rhi = RHI::Instance();
        ConfigSystem& configSystem = Singleton<ConfigSystem>::Instance();

        const std::filesystem::path noneEffectShaderPath = configSystem.GetHLSLShaderFolder() / "PostProcessNone.hlsl";

        ShaderList list = {};
        list.m_VS = rhi.CreateShader(ShaderType::VERTEX_SHADER, noneEffectShaderPath, "VS");
        list.m_PS = rhi.CreateShader(ShaderType::PIXEL_SHADER, noneEffectShaderPath, "PS");

        m_noneEffect = rhi.CreateMaterial("PostProcess_None", MaterialType::POST_PROCESS, list);
        m_noneEffect->GetDepthStencilState()->SetDepthEnable(false);
        m_noneEffect->GetRasterizerState()->SetCullMode(CullMode::NONE);
    }

    void PostProcessPass::CreateCanvas()
    {
        RHI& rhi = RHI::Instance();

        const VertexData vertex0 = { { -1.0f, 1.0f, 0.0f }, {}, {}, { 0.0f, 0.0f } };
        const VertexData vertex1 = { { -1.0f, -3.0f, 0.0f }, {}, {}, { 0.0f, 2.0f } };
        const VertexData vertex2 = { { 3.0f, 1.0f, 0.0f }, {}, {}, { 2.0f, 0.0f } };

        VertexData vertexBuffer[3] = { vertex0, vertex1, vertex2 };
        uint32 indexBuffer[3] = { 0, 1, 2 };

        Ref<MeshRawData> meshRawData = MakeRef<MeshRawData>();
        Ref<SubmeshRawData>& submeshRawData = meshRawData->m_submeshRawDatas.emplace_back(MakeRef<SubmeshRawData>());

        RawData& vertexRawData = submeshRawData->m_vertexBuffer;
        RawData& indexRawData = submeshRawData->m_indexBuffer;

        vertexRawData.m_buffer = std::make_unique<byte[]>(sizeof(vertexBuffer));
        memcpy(vertexRawData.m_buffer.get(), vertexBuffer, sizeof(vertexBuffer));
        vertexRawData.m_numElements = _countof(vertexBuffer);
        vertexRawData.m_stride = sizeof(VertexData);

        indexRawData.m_buffer = std::make_unique<byte[]>(sizeof(indexBuffer));
        memcpy(indexRawData.m_buffer.get(), indexBuffer, sizeof(indexBuffer));
        indexRawData.m_numElements = _countof(indexBuffer);
        indexRawData.m_stride = sizeof(uint32);

        submeshRawData->primitiveType = PrimitiveType::TRIANGLE;

        Ref<Mesh> canvasMesh = rhi.CreateMesh(meshRawData);
        m_canvas = rhi.CreateRenderItem(canvasMesh);
    }
}