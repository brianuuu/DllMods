#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgExitedExternalControl : public Hedgehog::Universe::MessageTypeSet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x168086C);

		MsgExitedExternalControl() {}
	};
		
	BB_ASSERT_SIZEOF(MsgExitedExternalControl, 0x10);
}