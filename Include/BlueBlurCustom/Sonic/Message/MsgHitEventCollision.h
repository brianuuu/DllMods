#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgHitEventCollision : public Hedgehog::Universe::MessageTypeSet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x1682064);

		Hedgehog::Base::CStringSymbol m_Symbol;
		int m_ID2;

		// Default constructor, most hit/leave events don't require IDs at all.
		MsgHitEventCollision() : m_Symbol(), m_ID2(0) {}

		MsgHitEventCollision(Hedgehog::Base::CStringSymbol symbol, int id2) : m_Symbol(symbol), m_ID2(id2) {}
	};
		
	BB_ASSERT_OFFSETOF(MsgHitEventCollision, m_Symbol, 0x10);
	BB_ASSERT_OFFSETOF(MsgHitEventCollision, m_ID2, 0x14);
	BB_ASSERT_SIZEOF(MsgHitEventCollision, 0x18);
}