#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgIsExternalControl : public Hedgehog::Universe::MessageTypeGet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x01680B4C);

		bool IsTrue = false;
	};
	
	BB_ASSERT_SIZEOF(MsgIsExternalControl, 0x14);
}