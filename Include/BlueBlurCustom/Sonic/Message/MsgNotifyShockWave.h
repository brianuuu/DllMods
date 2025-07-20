#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgNotifyShockWave : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1681E04);

        hh::math::CVector m_Position;
        float m_Duration;
		
        MsgNotifyShockWave
        (
            hh::math::CVector in_Position,
            float in_Duration
        )
            : m_Position(in_Position)
            , m_Duration(in_Duration)
        {}
    };

    BB_ASSERT_OFFSETOF(MsgNotifyShockWave, m_Position, 0x10);
    BB_ASSERT_OFFSETOF(MsgNotifyShockWave, m_Duration, 0x20);
    BB_ASSERT_SIZEOF(MsgNotifyShockWave, 0x30);
}