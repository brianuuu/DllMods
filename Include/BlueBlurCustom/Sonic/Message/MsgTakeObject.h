#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgTakeObject : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1680D94);
		
		uint32_t m_Type;
		
		MsgTakeObject
        (
            uint32_t in_Type
        ) 
            : m_Type(in_Type)
        {}
    };

	BB_ASSERT_OFFSETOF(MsgTakeObject, m_Type, 0x10);
    BB_ASSERT_SIZEOF(MsgTakeObject, 0x14);
}