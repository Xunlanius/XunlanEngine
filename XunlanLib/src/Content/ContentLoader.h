#pragma once

#if !defined SHIPPING

#include "Common/Common.h"

namespace Xunlan
{
    class ContentLoader
    {
    public:

        static bool LoadGame();
        static void UnloadGame();
    };
}

#endif