#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgLeaveEventCollision : public Hedgehog::Universe::MessageTypeSet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x168207C);

		int m_ID;

		MsgLeaveEventCollision() : m_ID(0) {}
		MsgLeaveEventCollision(int id) : m_ID(id) {}
	};
	
	BB_ASSERT_OFFSETOF(MsgLeaveEventCollision, m_ID, 0x10);
	BB_ASSERT_SIZEOF(MsgLeaveEventCollision, 0x14);
}