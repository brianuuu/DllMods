#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgGetItemType : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1680E68);

		size_t m_Type;
		bool m_ShouldChangeDirection; // always false?
        BB_INSERT_PADDING(0x3);
		hh::math::CVector m_Direction;

        MsgGetItemType
        (
            size_t in_Type
        ) 
            : m_Type(in_Type)
            , m_ShouldChangeDirection(false)
            , m_Direction(hh::math::CVector::UnitX())
        {}
    };

    BB_ASSERT_OFFSETOF(MsgGetItemType, m_Type, 0x10);
    BB_ASSERT_OFFSETOF(MsgGetItemType, m_ShouldChangeDirection, 0x14);
    BB_ASSERT_OFFSETOF(MsgGetItemType, m_Direction, 0x20);
    BB_ASSERT_SIZEOF(MsgGetItemType, 0x30);
}