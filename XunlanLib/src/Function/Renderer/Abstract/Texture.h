#pragma once

#include "src/Common/Common.h"
#include "RenderContext.h"

namespace Xunlan
{
    enum class TextureFormat : uint32
    {
        R8G8B8,
        R8G8B8A8,

        COUNT,
    };

    enum class TextureType : uint32
    {
        Image,
        RenderTarget,
        DepthBuffer,
    };

    class RawTexture final
    {
    public:

        explicit RawTexture(uint32 width, uint32 height, TextureFormat format, const void* data);
        DISABLE_COPY(RawTexture)
        DISABLE_MOVE(RawTexture)
        ~RawTexture() = default;

    public:

        uint32 GetWidth() const { return m_width; }
        uint32 GetHeight() const { return m_height; }
        TextureFormat GetFormat() const { return m_format; }
        uint32 GetPixelSize() const;
        void* GetData() const { return m_data.get(); }

    private:

        uint32 m_width;
        uint32 m_height;
        TextureFormat m_format;
        std::unique_ptr<byte[]> m_data;
    };

    class Texture
    {
    protected:

        explicit Texture(TextureType type, uint32 width, uint32 height)
            : m_type(type), m_width(width), m_height(height) {}
        DISABLE_COPY(Texture)
        DISABLE_MOVE(Texture)
        virtual ~Texture() = default;

    public:

        TextureType GetType() const { return m_type; }
        uint32 GetWidth() const { return m_width; }
        uint32 GetHeight() const { return m_height; }

        virtual uint32 GetHeapIndex() const = 0;

    protected:

        TextureType m_type;
        uint32 m_width;
        uint32 m_height;
    };

    class ImageTexture : public Texture
    {
    protected:

        explicit ImageTexture(const CRef<RawTexture>& rawTexture);

    public:

        TextureFormat GetFormat() const { return m_format; }
        uint32 GetPixelSize() const;

    protected:

        TextureFormat m_format;
    };

    class RenderTarget : public Texture
    {
    protected:

        explicit RenderTarget(uint32 width, uint32 height)
            : Texture(TextureType::RenderTarget, width, height) {}

    public:

        virtual void Resize(uint32 width, uint32 height) { m_width = width; m_height = height; }
    };

    class DepthBuffer : public Texture
    {
    protected:

        explicit DepthBuffer(uint32 width, uint32 height)
            : Texture(TextureType::DepthBuffer, width, height) {}

    public:

        virtual void Resize(uint32 width, uint32 height) { m_width = width; m_height = height; }
    };
}