#pragma once

#include "src/Common/Common.h"

namespace Xunlan
{
    enum class ShaderType : uint32
    {
        VERTEX_SHADER,
        HULL_SHADER,
        DOMAIN_SHADER,
        GEOMETRY_SHADER,
        PIXEL_SHADER,
        COMPUTE_SHADER,
        AMPLIFICATION_SHADER,
        MESH_SHADER,

        COUNT,
    };

    class Shader
    {
    protected:

        explicit Shader(ShaderType type) : m_type(type) {}

    public:

        virtual ~Shader() = default;

    protected:

        ShaderType m_type;
    };
}