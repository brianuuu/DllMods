#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgGetEnemyType : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1681D0C);

		uint32_t* m_pType;

        MsgGetEnemyType(uint32_t* in_pType) : m_pType(in_pType) {}
        MsgGetEnemyType(uint32_t& in_rType) : m_pType(&in_rType) {}
    };

    BB_ASSERT_OFFSETOF(MsgGetEnemyType, m_pType, 0x10);
    BB_ASSERT_SIZEOF(MsgGetEnemyType, 0x14);
}