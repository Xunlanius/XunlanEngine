#pragma once

#include "src/Common/Common.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"
#include "CBuffer.h"
#include "DataBuffer.h"

#include <string>
#include <unordered_map>

namespace Xunlan
{
    struct ShaderInitDesc final
    {
        bool m_createVS = false;
        bool m_createPS = false;
    };

    class Shader
    {
    protected:

        explicit Shader(const std::string& name, const ShaderInitDesc& desc);
        DISABLE_COPY(Shader)
        DISABLE_MOVE(Shader)
        virtual ~Shader() = default;

    public:

        const std::string& GetName() const { return m_name; }
        Ref<RasterizerState> GetRasterizerState() const { return m_rasterizerState; }
        Ref<DepthStencilState> GetDepthStencilState() const { return m_depthStencilState; }

        bool ContainVS() const { return m_containVS; }
        bool ContainPS() const { return m_containPS; }

        virtual bool HasParam(const std::string& name) const = 0;
        virtual bool SetParam(const std::string& name, CRef<CBuffer> cBuffer) = 0;
        virtual bool SetParam(const std::string& name, CRef<DataBuffer> buffer) = 0;

    protected:

        std::string m_name;

        Ref<RasterizerState> m_rasterizerState;
        Ref<DepthStencilState> m_depthStencilState;

        bool m_containVS;
        bool m_containPS;
    };
}