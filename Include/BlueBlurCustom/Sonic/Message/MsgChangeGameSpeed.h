#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgChangeGameSpeed : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x16819D4);

        float m_GameSpeed;
        size_t m_Mode; // 1 = normal, 0 = slowed

        MsgChangeGameSpeed
        (
            float in_GameSpeed = 1.0f,
            size_t in_Mode = 1
        ) 
            : m_GameSpeed(in_GameSpeed)
            , m_Mode(in_Mode)
        {}
    };

    BB_ASSERT_OFFSETOF(MsgChangeGameSpeed, m_GameSpeed, 0x10);
    BB_ASSERT_OFFSETOF(MsgChangeGameSpeed, m_Mode, 0x14);
    BB_ASSERT_SIZEOF(MsgChangeGameSpeed, 0x18);
}