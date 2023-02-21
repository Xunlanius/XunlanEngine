#pragma once

#include <cstdint>
#include <cassert>
#include "Common/DataTypes.h"

namespace Xunlan
{
    class ID final
    {
    public:

        using id_type = uint32;
        using generation_type = byte;

    private:

        static constexpr uint32 ms_generationNumBits = sizeof(generation_type) * 8;
        static constexpr uint32 ms_indexNumBits = sizeof(id_type) * 8 - ms_generationNumBits;

        static constexpr id_type ms_generationMask = ((id_type)1 << ms_generationNumBits) - 1;
        static constexpr id_type ms_indexMask = ((id_type)1 << ms_indexNumBits) - 1;

        static_assert(sizeof(ID::generation_type) < sizeof(ID::id_type));

    public:

        static constexpr bool IsValid(id_type id) { return id != INVALID_ID; }
        static constexpr id_type GetGenerationPart(id_type id) { return (id >> ms_indexNumBits) & ms_generationMask; }
        static constexpr id_type GetIndexPart(id_type id) { return id & ms_indexMask; }
        static constexpr id_type GetID(id_type generationPart, id_type indexPart)
        {
            // The "generation" part and the "index" part should not exceed
            assert(generationPart < ms_generationMask);
            assert(indexPart < ms_indexMask);
            return (generationPart << ms_indexNumBits) | indexPart;
        }

        static constexpr id_type INVALID_ID = (id_type)-1;

        //static constexpr id_type ms_invalidID = (id_type)-1;
        static constexpr uint32 ms_maxNumIdleIDs = 1024;
    };
}