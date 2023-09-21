#include "RenderContext.h"

namespace Xunlan
{
    void RenderContext::BindParams(Ref<Shader> shader) const
    {
        for (const auto& [name, cBuffer] : m_paramCBVs)
        {
            shader->SetParam(name, cBuffer);
        }

        for (const auto& [name, buffer] : m_paramSRVs)
        {
            shader->SetParam(name, buffer);
        }
    }
}