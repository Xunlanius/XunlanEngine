#pragma once

#include "src/Function/Renderer/Abstract/RenderContext.h"
#include "DX12Common.h"

namespace Xunlan::DX12
{
    class DX12RenderContext final : public RenderContext
    {
    public:

        explicit DX12RenderContext(GraphicsCommandList* cmdList) : m_cmdList(cmdList) {}

    public:

        GraphicsCommandList* m_cmdList;
    };
}