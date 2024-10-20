#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgNotifyShockWave : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1681E04);
		
		float m_Duration;
		
		MsgNotifyShockWave
        (
            float in_Duration
        ) 
            : m_Duration(in_Duration)
        {}
    };

	BB_ASSERT_OFFSETOF(MsgNotifyShockWave, m_Duration, 0x10);
    BB_ASSERT_SIZEOF(MsgNotifyShockWave, 0x14);
}