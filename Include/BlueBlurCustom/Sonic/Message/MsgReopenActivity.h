#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgReopenActivity : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x01682000);
		
        MsgReopenActivity() {}
    };

    BB_ASSERT_SIZEOF(MsgReopenActivity, 0x10);
}