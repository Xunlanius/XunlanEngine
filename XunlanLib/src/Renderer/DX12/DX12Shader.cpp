#include "DX12Shader.h"
#include "Content/ContentLoader.h"

namespace Xunlan::Graphics::DX12::Shader
{
    namespace
    {
        std::unique_ptr<byte[]> g_shadersBlob = nullptr;
        D3D12_SHADER_BYTECODE g_engineShaders[(uint32)EngineShader::Count] = {};

        bool LoadEngineShaders()
        {
            assert(!g_shadersBlob);

            uint64 byteSize = 0;
            if (!ContentLoader::LoadEngineShaders(g_shadersBlob, byteSize)) return false;
            assert(byteSize > 0);

            uint64 offset = 0;
            uint32 index = 0;
            while (offset < byteSize)
            {
                if (index >= (uint32)EngineShader::Count) { assert(false); return false; }

                D3D12_SHADER_BYTECODE& shader = g_engineShaders[index++];
                shader.BytecodeLength = *(decltype(shader.BytecodeLength)*)&g_shadersBlob[offset];
                offset += sizeof(shader.BytecodeLength);
                shader.pShaderBytecode = &g_shadersBlob[offset];
                offset += shader.BytecodeLength;
            }

            if (offset != byteSize || index != (uint32)EngineShader::Count) { assert(false); return false; }

            return true;
        }
    }

    bool Initialize()
    {
        return LoadEngineShaders();
    }
    void Shutdown()
    {
        for (uint32 i = 0; i < (uint32)EngineShader::Count; ++i)
        {
            g_engineShaders[i] = {};
        }
        g_shadersBlob.reset();
    }

    D3D12_SHADER_BYTECODE GetEngineShaderByteCode(EngineShader id)
    {
        if (id >= EngineShader::Count) { assert(false); return {}; }
        return g_engineShaders[(uint32)id];
    }
}