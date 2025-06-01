#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgLeaveEventCollision : public Hedgehog::Universe::MessageTypeSet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x168207C);

		Hedgehog::Base::CStringSymbol m_Symbol;

		MsgLeaveEventCollision() : m_Symbol() {}
		MsgLeaveEventCollision(Hedgehog::Base::CStringSymbol const& symbol) : m_Symbol(symbol) {}
	};
	
	BB_ASSERT_OFFSETOF(MsgLeaveEventCollision, m_Symbol, 0x10);
	BB_ASSERT_SIZEOF(MsgLeaveEventCollision, 0x14);
}