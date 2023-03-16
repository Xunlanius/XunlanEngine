#pragma once

#include "Renderer/DX12/DX12Common.h"

namespace Xunlan::Graphics::DX12::Asset
{
    namespace Submesh
    {
        ID Add(const byte*& data);
        void Remove(ID id);
    }
}