#include "Shader.h"
#include "RHI.h"

namespace Xunlan
{
    Shader::Shader(const std::string& name, const ShaderInitDesc& desc) :
        m_name(name),
        m_containVS(desc.m_createVS),
        m_containPS(desc.m_createPS)
    {
        RHI& rhi = RHI::Instance();

        m_rasterizerState = rhi.CreateRasterizerState({});
        m_depthStencilState = rhi.CreateDepthStencilState();
    }
}