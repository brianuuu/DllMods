#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgExitedHangOn : public Hedgehog::Universe::MessageTypeSet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x1680DB0);

		MsgExitedHangOn() {}
	};
		
	BB_ASSERT_SIZEOF(MsgExitedHangOn, 0x10);
}