#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgRestartStage : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1681FA0);

        uint32_t m_Flag; // 0: player restart, 1: death, 2: ???

        MsgRestartStage(uint32_t in_Flag) : m_Flag(in_Flag) {}
    };

    BB_ASSERT_OFFSETOF(MsgRestartStage, m_Flag, 0x10);
    BB_ASSERT_SIZEOF(MsgRestartStage, 0x14);
}