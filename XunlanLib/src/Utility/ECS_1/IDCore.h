#pragma once

#include "Common/Common.h"

namespace Xunlan
{
    using EntityID = ID::id_type;
    using ComponentID = ID::id_type;
    using ResourceID = ID::id_type;

    struct Component {};
    struct Resource {};

    template<typename Category>
    class IDGetter final
    {
    public:

        template<typename Type>
        static ID::id_type Get()
        {
            static ID::id_type id = ms_currIndex++;
            return id;
        }

    private:

        static inline ID::id_type ms_currIndex = 0;
    };

    class EntityGenerator final
    {
    public:

        static EntityID Generate() { return ms_currID++; }

    private:

        static inline EntityID ms_currID = 0;
    };
}