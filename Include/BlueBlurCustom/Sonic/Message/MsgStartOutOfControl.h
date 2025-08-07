#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgStartOutOfControl : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x167FCF4);
		
		float m_Time;
		bool m_Field14;

        MsgStartOutOfControl
		(
			float in_Time,
			bool in_Field14 = false
		) 
		: m_Time(in_Time) 
		, m_Field14(m_Field14) 
		{
		}
    };

    BB_ASSERT_OFFSETOF(MsgStartOutOfControl, m_Time, 0x10);
    BB_ASSERT_OFFSETOF(MsgStartOutOfControl, m_Field14, 0x14);
    BB_ASSERT_SIZEOF(MsgStartOutOfControl, 0x18);
}