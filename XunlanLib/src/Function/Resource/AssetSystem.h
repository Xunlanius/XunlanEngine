#pragma once

#include "src/Common/Common.h"
#include "src/Utility/Reflection/Serializer.h"

#include <filesystem>

namespace Xunlan
{
    class AssetSystem final
    {
    public:

        template<typename T>
        static void LoadAsset(const std::filesystem::path& filePath, T& asset)
        {
            Reflection::BinaryStream stream;
            stream.Load(filePath);
            stream >> asset;
        }
    };
}