#pragma once

#include "src/Function/Renderer/RenderCommon.h"
#include "RenderContext.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"
#include "CBuffer.h"
#include "Material.h"
#include "RenderItem.h"
#include <filesystem>

namespace Xunlan
{
    class RHI
    {
    protected:

        explicit RHI(Platform platform) : m_platform(platform) {}
        DISABLE_COPY(RHI)
        DISABLE_MOVE(RHI)
        virtual ~RHI() = default;

    public:

        static RHI& Create(Platform platform);
        static RHI& Instance() { return *ms_instance; }
        static void Shutdown() { delete ms_instance; }

        Platform GetPlatform() const { return m_platform; }

        virtual void Flush() = 0;

        virtual uint32 GetWidth() = 0;
        virtual uint32 GetHeight() = 0;
        virtual void Resize(uint32 width, uint32 height) = 0;

        virtual Ref<RenderContext> CreateRenderContext() = 0;
        virtual void Execute(Ref<RenderContext>& context) = 0;
        virtual void Present() = 0;

        virtual void SetRenderTarget(const Ref<RenderContext>& context, const CRef<RenderTarget>& renderTarget) = 0;
        virtual void ClearRenderTarget(const Ref<RenderContext>& context, const CRef<RenderTarget>& renderTarget) = 0;
        virtual void ResetRenderTarget(const Ref<RenderContext>& context, const CRef<RenderTarget>& renderTarget) = 0;
        virtual void SetViewport(const Ref<RenderContext>& context, uint32 x, uint32 y, uint32 width, uint32 height) = 0;

        Ref<Mesh> CreateMesh(const std::filesystem::path& path);
        virtual Ref<Mesh> CreateMesh(const CRef<MeshRawData>& meshRawData) = 0;
        virtual Ref<Shader> CreateShader(ShaderType type, const std::filesystem::path& path, const std::string& functionName) = 0;
        Ref<ImageTexture> CreateImageTexture(const std::filesystem::path& path);
        virtual Ref<ImageTexture> CreateImageTexture(const CRef<RawTexture>& rawTexture) = 0;
        virtual Ref<RenderTarget> CreateRenderTarget(uint32 width, uint32 height, RenderTargetUsage usage) = 0;
        virtual Ref<RasterizerState> CreateRasterizerState(const RasterizerStateDesc& desc) = 0;
        virtual Ref<DepthStencilState> CreateDepthStencilState() = 0;
        virtual Ref<CBuffer> CreateConstantBuffer(CBufferType type, uint32 size) = 0;
        virtual Ref<Material> CreateMaterial(const std::string& name, MaterialType type, const ShaderList& shaderList) = 0;
        virtual Ref<RenderItem> CreateRenderItem(const Ref<Mesh>& mesh) = 0;
        virtual Ref<RenderItem> CreateRenderItem(const Ref<Mesh>& mesh, const std::vector<Ref<Material>>& materials) = 0;

    protected:

        static inline RHI* ms_instance = nullptr;

        const Platform m_platform;
    };
}