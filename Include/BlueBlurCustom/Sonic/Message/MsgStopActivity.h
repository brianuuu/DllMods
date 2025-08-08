#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgStopActivity : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1681FF0);
		
		bool m_NoDamage;

        MsgStopActivity
		(
			bool in_NoDamage = true
		) 
		: m_NoDamage(in_NoDamage)
		{
		}
    };

    BB_ASSERT_OFFSETOF(MsgStopActivity, m_NoDamage, 0x10);
    BB_ASSERT_SIZEOF(MsgStopActivity, 0x14);
}