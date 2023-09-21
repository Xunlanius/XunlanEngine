#pragma once

#include "src/Common/Common.h"
#include "CBuffer.h"
#include "DataBuffer.h"
#include "Shader.h"

#include <string>
#include <unordered_map>

namespace Xunlan
{
    class RenderContext
    {
    protected:

        RenderContext() = default;
        DISABLE_COPY(RenderContext)
        DISABLE_MOVE(RenderContext)
        virtual ~RenderContext() = default;

    public:

        void SetParam(const std::string& name, CRef<CBuffer> cBuffer) { m_paramCBVs[name] = cBuffer; }
        void SetParam(const std::string& name, CRef<DataBuffer> buffer) { m_paramSRVs[name] = buffer; }

        void BindParams(Ref<Shader> shader) const;

    protected:

        std::unordered_map<std::string, CRef<CBuffer>> m_paramCBVs;
        std::unordered_map<std::string, CRef<DataBuffer>> m_paramSRVs;
    };
}