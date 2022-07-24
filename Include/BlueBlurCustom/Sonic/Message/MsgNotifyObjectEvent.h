#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgNotifyObjectEvent : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x16811C0);

		size_t m_Event;
		bool m_Field20; // always false?
        BB_INSERT_PADDING(0x3);

        // the same order as sub_4F9E90
        MsgNotifyObjectEvent
        (
            size_t in_Event
        ) 
            : m_Event(in_Event)
            , m_Field20(false)
        {}
    };

    BB_ASSERT_OFFSETOF(MsgNotifyObjectEvent, m_Event, 0x10);
    BB_ASSERT_OFFSETOF(MsgNotifyObjectEvent, m_Field20, 0x14);
    BB_ASSERT_SIZEOF(MsgNotifyObjectEvent, 0x18);
}