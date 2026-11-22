#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgApplyImpulse : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1680D84);

        Hedgehog::Math::CVector m_Position = Hedgehog::Math::CVector::Zero();
        Hedgehog::Math::CVector m_Impulse = Hedgehog::Math::CVector::Zero();
		float m_OutOfControl = 0.0f;
		float m_Field34 = 0.0f;
		uint32_t m_ImpulseType = 0;
		float m_KeepVelocityTime = 0.0f;
		bool m_NotRelative = true;
		bool m_SnapPosition = false;
		bool m_IsChangePath = false;
		bool m_IsChangeCameraWhenChangePath = false;
		bool m_Field44 = false;
		bool m_PathInterpolate = false;
		BB_INSERT_PADDING(0xA);
        Hedgehog::Math::CVector m_Field50;
		float m_Field60 = -1.0f;
		BB_INSERT_PADDING(0xC);

        MsgApplyImpulse
		(
			const Hedgehog::Math::CVector& in_rPosition,
			const Hedgehog::Math::CVector& in_rImpulse,
			uint32_t in_ImpulseType,
			float in_OutOfControl = 0.0f
		) 
		: m_Position(in_rPosition) 
		, m_Impulse(in_rImpulse) 
		, m_ImpulseType(in_ImpulseType)
		, m_OutOfControl(in_OutOfControl) 
		{}
    };

    BB_ASSERT_OFFSETOF(MsgApplyImpulse, m_Position, 0x10);
    BB_ASSERT_OFFSETOF(MsgApplyImpulse, m_Impulse, 0x20);
    BB_ASSERT_OFFSETOF(MsgApplyImpulse, m_OutOfControl, 0x30);
    BB_ASSERT_OFFSETOF(MsgApplyImpulse, m_ImpulseType, 0x38);
    BB_ASSERT_OFFSETOF(MsgApplyImpulse, m_KeepVelocityTime, 0x3C);
    BB_ASSERT_OFFSETOF(MsgApplyImpulse, m_NotRelative, 0x40);
    BB_ASSERT_OFFSETOF(MsgApplyImpulse, m_SnapPosition, 0x41);
    BB_ASSERT_OFFSETOF(MsgApplyImpulse, m_IsChangePath, 0x42);
    BB_ASSERT_OFFSETOF(MsgApplyImpulse, m_IsChangeCameraWhenChangePath, 0x43);
    BB_ASSERT_OFFSETOF(MsgApplyImpulse, m_PathInterpolate, 0x45);
    BB_ASSERT_SIZEOF(MsgApplyImpulse, 0x70);
}