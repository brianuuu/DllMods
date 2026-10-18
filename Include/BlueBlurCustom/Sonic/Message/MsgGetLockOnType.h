#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

enum LockOnType
{
    Enemy = 0x1,
    ChaosSpear = 0x2,
};

namespace Sonic::Message
{
    class MsgGetLockOnType : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE("MsgGetLockOnType");
		
		uint32_t* m_pType;

        MsgGetLockOnType(uint32_t* in_pType) : m_pType(in_pType) {}
        MsgGetLockOnType(uint32_t& in_rType) : m_pType(&in_rType) {}
    };

    BB_ASSERT_OFFSETOF(MsgGetLockOnType, m_pType, 0x10);
    BB_ASSERT_SIZEOF(MsgGetLockOnType, 0x14);
}