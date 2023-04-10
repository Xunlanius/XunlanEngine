#pragma once

#include "src/Common/DataTypes.h"
#include <fstream>
#include <filesystem>

namespace Xunlan::IO
{
    inline bool ReadFile(std::filesystem::path path, std::unique_ptr<byte[]>& buffer, uint64& byteSize)
    {
        if (!std::filesystem::exists(path)) return false;

        byteSize = std::filesystem::file_size(path);
        if (byteSize == 0) return false;

        buffer = std::make_unique<byte[]>(byteSize);

        std::ifstream fileStream(path, std::ios::in | std::ios::binary);
        if (!fileStream || !fileStream.read((char*)buffer.get(), byteSize))
        {
            fileStream.close();
            return false;
        }

        fileStream.close();
        return true;
    }
}