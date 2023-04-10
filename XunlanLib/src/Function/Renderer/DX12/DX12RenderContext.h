#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/RenderContext.h"

namespace Xunlan::DX12
{
    class DX12RenderContext final : public RenderContext
    {
    public:

        DX12RenderContext(GraphicsCommandList* cmdList) : m_cmdList(cmdList) {}

    public:

        GraphicsCommandList* m_cmdList;
    };
}