#pragma once

#include <cstdint>
#include <cassert>

namespace Xunlan
{
    class ID final
    {
    public:

        using id_type = uint32_t;
        using generation_type = uint8_t;

    private:

        static constexpr uint32_t ms_generationNumBits = sizeof(generation_type) * 8;
        static constexpr uint32_t ms_indexNumBits = sizeof(id_type) * 8 - ms_generationNumBits;

        static constexpr id_type ms_generationMask = ((id_type)1 << ms_generationNumBits) - 1;
        static constexpr id_type ms_indexMask = ((id_type)1 << ms_indexNumBits) - 1;

        static_assert(sizeof(ID::generation_type) < sizeof(ID::id_type));

    public:

        static constexpr bool IsValid(id_type id) { return id != ms_invalidID; }
        static constexpr id_type GetGenerationPart(id_type id) { return (id >> ms_indexNumBits) & ms_generationMask; }
        static constexpr id_type GetIndexPart(id_type id) { return id & ms_indexMask; }
        static constexpr id_type GetID(id_type generationPart, id_type indexPart)
        {
            // The "generation" part and the "index" part should not exceed
            assert(generationPart < ms_generationMask);
            assert(indexPart < ms_indexMask);
            return (generationPart << ms_indexNumBits) | indexPart;
        }

    public:

        static constexpr id_type ms_invalidID = (id_type)-1;
        static constexpr uint32_t ms_maxNumIdleIDs = 1024;
    };

    class IDModel
    {
    public:

        constexpr IDModel()
            : m_id(ID::ms_invalidID) {}
        constexpr explicit IDModel(ID::id_type id)
            : m_id(id) {}
        virtual ~IDModel() = default;

    public:

        operator ID::id_type() { return m_id; }

        ID::id_type GetID() const { return m_id; }
        ID::id_type GetGeneration() const { return ID::GetGenerationPart(m_id); }
        ID::id_type GetIndex() const { return ID::GetIndexPart(m_id); }
        bool IsValid() const { return ID::IsValid(m_id); }

    protected:

        ID::id_type m_id;
    };

//#if defined _DEBUG
//    namespace Internal
//    {
//        class IDBase
//        {
//        public:
//
//            constexpr IDBase(ID::id_type id)
//                : id(id) {}
//            constexpr operator ID::id_type() const { return id; }
//
//        private:
//
//            ID::id_type id;
//        };
//    }
//
//    #define DEFINE_TYPED_ID(name)                           \
//            class name final : public Internal::IDBase      \
//            {                                               \
//            public:                                         \
//                                                            \
//                constexpr name(ID::id_type id)              \
//                    : Internal::IDBase(id) {}               \
//                constexpr name()                            \
//                : Internal::IDBase(0) {}                    \
//            };
//#else
//    #define DEFINE_TYPED_ID(name) typedef ID::id_type name;
//#endif
}