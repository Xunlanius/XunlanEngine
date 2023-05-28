#include "Texture.h"
#include <cassert>

namespace Xunlan
{
    RawTexture::RawTexture(uint32 width, uint32 height, TextureFormat format, const void* data)
        : m_width(width), m_height(height), m_format(format)
    {
        assert(m_format < TextureFormat::COUNT);

        const uint32 bufferSize = m_width * m_height * GetPixelSize();
        m_data = std::make_unique<byte[]>(bufferSize);
        memcpy(m_data.get(), data, bufferSize);
    }

    uint32 RawTexture::GetPixelSize() const
    {
        switch (m_format)
        {
            case TextureFormat::R8G8B8: return 3;
            case TextureFormat::R8G8B8A8: return 4;
            default: assert(false); return 0;
        }
    }

    ImageTexture::ImageTexture(const CRef<RawTexture>& rawTexture)
        : Texture(TextureType::IMAGE, rawTexture->GetWidth(), rawTexture->GetHeight()), m_format(rawTexture->GetFormat()) {}

    uint32 ImageTexture::GetPixelSize() const
    {
        switch (m_format)
        {
            case TextureFormat::R8G8B8: return 3;
            case TextureFormat::R8G8B8A8: return 4;
            default: assert(false); return 0;
        }
    }
}