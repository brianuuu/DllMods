#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgNotifySenseEvent : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1680190);

        uint32_t m_Field10;
		
        MsgNotifySenseEvent
        (
            uint32_t in_Field10 = 0
        )
            : m_Field10(in_Field10)
        {}
    };

    BB_ASSERT_OFFSETOF(MsgNotifySenseEvent, m_Field10, 0x10);
    BB_ASSERT_SIZEOF(MsgNotifySenseEvent, 0x14);
}