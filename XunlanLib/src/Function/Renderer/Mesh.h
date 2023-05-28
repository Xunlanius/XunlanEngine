#pragma once

#include "src/Common/Common.h"
#include "RenderBuffer.h"
#include <vector>

namespace Xunlan
{
    enum class PrimitiveType : uint32
    {
        UNKNOWN,
        POINT,
        LINE,
        TRIANGLE,
    };

    struct SubmeshRawData final
    {
        RawData m_vertexBuffer;
        RawData m_indexBuffer;

        PrimitiveType primitiveType;
        uint32 materialIndex;
    };

    struct MeshRawData final
    {
        std::vector<Ref<SubmeshRawData>> m_submeshRawDatas;
        uint32 m_numMaterials;
    };

    class Submesh
    {
    protected:

        explicit Submesh(const Ref<RenderBuffer>& verticesView, const Ref<RenderBuffer>& indicesView, PrimitiveType primitiveType, uint32 materialIndex)
            : m_verticesView(verticesView), m_indicesView(indicesView), m_primitiveType(primitiveType), m_materialIndex(materialIndex) {}
    
    public:

        virtual ~Submesh() = default;

        uint32 GetMaterialIndex() const { return m_materialIndex; }
        void SetMaterialIndex(uint32 value) { m_materialIndex = value; }
        Ref<RenderBuffer> GetVerticesView() const { return m_verticesView; }
        Ref<RenderBuffer> GetIndicesView() const { return m_indicesView; }

    protected:

        Ref<RenderBuffer> m_verticesView;
        Ref<RenderBuffer> m_indicesView;

        PrimitiveType m_primitiveType;
        uint32 m_materialIndex = 0;
    };

    class Mesh
    {
    protected:

        explicit Mesh(uint32 numMaterial) : m_numMaterials(numMaterial) {}

    public:

        virtual ~Mesh() = default;

    public:

        uint32 GetNumMaterials() const { return m_numMaterials; }
        void SetNumMaterials(uint32 value) { m_numMaterials = value; }
        const std::vector<Ref<Submesh>>& GetSubmeshes() const { return m_submeshes; }

        void AddSubmesh(const Ref<Submesh>& submeshView) { m_submeshes.push_back(submeshView); }

    protected:

        std::vector<Ref<Submesh>> m_submeshes;
        uint32 m_numMaterials = 0;
    };
}