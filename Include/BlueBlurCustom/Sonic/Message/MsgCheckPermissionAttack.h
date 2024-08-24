#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgCheckPermissionAttack : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1681D0C);

		bool m_Permitted;
        BB_INSERT_PADDING(0x3);

        MsgCheckPermissionAttack() 
            : m_Permitted(false)
        {}
    };

    BB_ASSERT_OFFSETOF(MsgCheckPermissionAttack, m_Permitted, 0x10);
    BB_ASSERT_SIZEOF(MsgCheckPermissionAttack, 0x14);
}