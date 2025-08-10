#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgMissionFinish : public Hedgehog::Universe::MessageTypeSet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x1681278);

		MsgMissionFinish() {}
	};
		
	BB_ASSERT_SIZEOF(MsgMissionFinish, 0x10);
}