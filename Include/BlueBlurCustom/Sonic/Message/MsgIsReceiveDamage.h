#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgIsReceiveDamage : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x167FFF4);

		uint32_t m_DamageType;
		uint32_t m_Field14;
		bool* m_pSuccess;

        MsgIsReceiveDamage
		(
			bool* in_pSuccess,
			uint32_t in_DamageType = 0,
			uint32_t in_Field14 = 0
		) 
		: m_pSuccess(in_pSuccess) 
		, m_DamageType(in_DamageType)
		, m_Field14(in_Field14)
		{}
		
        MsgIsReceiveDamage
		(
			bool& in_rSuccess,
			uint32_t in_DamageType = 0,
			uint32_t in_Field14 = 0
		) 
		: m_pSuccess(&in_rSuccess) 
		, m_DamageType(in_DamageType)
		, m_Field14(in_Field14)
		{}
    };

    BB_ASSERT_OFFSETOF(MsgIsReceiveDamage, m_DamageType, 0x10);
    BB_ASSERT_OFFSETOF(MsgIsReceiveDamage, m_Field14, 0x14);
    BB_ASSERT_OFFSETOF(MsgIsReceiveDamage, m_pSuccess, 0x18);
    BB_ASSERT_SIZEOF(MsgIsReceiveDamage, 0x1C);
}