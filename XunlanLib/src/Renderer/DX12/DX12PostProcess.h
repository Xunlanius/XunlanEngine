#pragma once

#include "Renderer/DX12/DX12Common.h"

namespace Xunlan::Graphics::DX12::PostProcess
{
    bool Initialize();
    void Shutdown();

    void PostProcess(GraphicsCommandList& cmdList, D3D12_CPU_DESCRIPTOR_HANDLE rtv);
}