#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
	class MsgDamage : public hh::fnd::MessageTypeSet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x01681E80);

		//Hedgehog::Base::SSymbolNode* m_DamageType {};
		uint32_t m_DamageType {};
		int m_DamageEffectID {};
		Hedgehog::Math::CVector m_DamagePosition {};
		Hedgehog::Math::CVector m_PositionB {};
		Hedgehog::Math::CVector m_Velocity {};
		int m_SuccessActorIDOverride {};

		MsgDamage(uint32_t in_DamageType, const Hedgehog::Math::CVector& position, const Hedgehog::Math::CVector& velocity)
		: m_DamageType(in_DamageType), m_DamageEffectID(0), m_DamagePosition(position), m_PositionB(position), m_Velocity(velocity),  m_SuccessActorIDOverride(0)
		{}

		MsgDamage(uint32_t in_DamageType, const Hedgehog::Math::CVector& position)
		: m_DamageType(in_DamageType), m_DamageEffectID(0), m_DamagePosition(position), m_PositionB(position), m_Velocity(Hedgehog::Math::CVector::Zero()),  m_SuccessActorIDOverride(0)
		{}
	};
	BB_ASSERT_SIZEOF(MsgDamage, 0x60);
}