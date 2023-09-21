#pragma once

#include "src/Function/Renderer/Abstract/Material.h"
#include "DX12Common.h"
#include "Shader/DX12Shader.h"

namespace Xunlan::DX12
{
    class DX12Material final : public Material
    {
    public:

        explicit DX12Material(Ref<Shader> shader)
            : Material(shader) {}

    public:

        virtual void Apply(Ref<RenderContext> context) const override;
    };
}