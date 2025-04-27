#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgDeactivate : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1681F0C);

        bool m_Flag;
        BB_INSERT_PADDING(0x3);

        MsgDeactivate() : m_Flag(true) {}
    };

    BB_ASSERT_OFFSETOF(MsgDeactivate, m_Flag, 0x10);
    BB_ASSERT_SIZEOF(MsgDeactivate, 0x14);
}