#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgIsWall : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1680DA4);

        bool* m_pIsWall;

        MsgIsWall(bool* in_pIsWall) : m_pIsWall(in_pIsWall) {}
        MsgIsWall(bool& in_rIsWall) : m_pIsWall(&in_rIsWall) {}
    };

    BB_ASSERT_OFFSETOF(MsgIsWall, m_pIsWall, 0x10);
    BB_ASSERT_SIZEOF(MsgIsWall, 0x14);
}