#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgGetPlayerType : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x0167FF00);
		
		enum class Type
		{
			Modern,
			Classic,
			Super
		} m_Type;
		
		MsgGetPlayerType() : m_Type(Type::Modern) {}
    };

	BB_ASSERT_OFFSETOF(MsgGetPlayerType, m_Type, 0x10);
    BB_ASSERT_SIZEOF(MsgGetPlayerType, 0x14);
}