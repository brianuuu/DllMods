#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgStopActivity : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1681FF0);
		
		bool m_KeepVelocity;

        MsgStopActivity
		(
			bool in_KeepVelocity = true
		) 
		: m_KeepVelocity(in_KeepVelocity) 
		{
		}
    };

    BB_ASSERT_OFFSETOF(MsgStopActivity, m_KeepVelocity, 0x10);
    BB_ASSERT_SIZEOF(MsgStopActivity, 0x14);
}