#include "ID.h"

namespace Xunlan
{
    constexpr uint32_t ID::ms_generationNumBits;
    constexpr uint32_t ID::ms_indexNumBits;

    constexpr ID::id_type ID::ms_generationMask;
    constexpr ID::id_type ID::ms_indexMask;

    constexpr ID::id_type ID::ms_invalidID;
    constexpr uint32_t ID::ms_maxNumIdleIDs;
}