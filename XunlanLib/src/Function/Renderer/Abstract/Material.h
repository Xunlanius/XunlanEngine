#pragma once

#include "src/Common/Common.h"
#include "Shader.h"
#include "CBuffer.h"
#include "RenderContext.h"

#include <string>

namespace Xunlan
{
    class Material
    {
    protected:

        explicit Material(Ref<Shader> shader);
        DISABLE_COPY(Material)
        DISABLE_MOVE(Material)
        virtual ~Material() = default;

    public:

        Ref<Shader> GetShader() const { return m_shader; }
        Ref<CBuffer> GetCBPBR() const { return m_cbPBR; }
        CB::PBR* GetPBRData() const { return m_cbPBR->GetData<CB::PBR>(); }

        virtual void Apply(Ref<RenderContext> context) const = 0;

    protected:

        Ref<Shader> m_shader;
        Ref<CBuffer> m_cbPBR;
    };
}