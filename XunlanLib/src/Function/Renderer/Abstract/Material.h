#pragma once

#include "src/Common/Common.h"
#include "Shader.h"
#include "Texture.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"
#include "CBuffer.h"

#include <string>
#include <array>

namespace Xunlan
{
    enum class TextureCategory : uint32
    {
        Albedo,
        Roughness,
        Metallic,
        Normal,

        Count,
    };

    enum class MaterialType : uint32
    {
        MeshRenderer,
        PostProcess,
        ShadowMapping,
    };

    struct ShaderList final
    {
        CRef<Shader> m_VS;
        CRef<Shader> m_PS;
    };

    class Material
    {
    protected:

        explicit Material(const std::string& name, MaterialType type, const ShaderList& shaderList);
        DISABLE_COPY(Material)
        DISABLE_MOVE(Material)
        virtual ~Material() = default;

    public:

        const std::string& GetName() const { return m_name; }
        MaterialType GetType() const { return m_type; }
        Ref<RasterizerState> GetRasterizerState() const { return m_rasterizerState; }
        Ref<DepthStencilState> GetDepthStencilState() const { return m_depthStencilState; }

        CRef<Texture> GetTexture(TextureCategory category) const { return m_textureParams[(uint32)category]; }
        void SetTexture(TextureCategory category, CRef<Texture> value) { assert(value); m_textureParams[(uint32)category] = value; }

        virtual void Apply(Ref<RenderContext> context) const = 0;

    protected:

        std::string m_name;
        MaterialType m_type;

        CRef<Shader> m_VS;
        CRef<Shader> m_PS;

        Ref<RasterizerState> m_rasterizerState;
        Ref<DepthStencilState> m_depthStencilState;

        Ref<CBuffer> m_perMaterial;
        Ref<CBuffer> m_textureIndices;

        std::array<CRef<Texture>, (size_t)TextureCategory::Count> m_textureParams;
    };
}