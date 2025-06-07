#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgStageClear : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1681AC8);

        bool m_Success;
        BB_INSERT_PADDING(0x3);

        MsgStageClear(bool in_Success) : m_Success(in_Success) {}
    };

    BB_ASSERT_OFFSETOF(MsgStageClear, m_Success, 0x10);
    BB_ASSERT_SIZEOF(MsgStageClear, 0x14);
}