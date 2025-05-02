#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgStartHangOn : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x167FBA8);
		
		enum class Type // sub_E461D0
		{
			Pulley,
			UpReel,
			SphHelicopter,
			SphRocket,
			SszUpDownPoleNoMove,
			SszUpDownPole,
			EucClockTower,
		};

		Type m_Type;
		boost::shared_ptr<Hedgehog::Mirage::CMatrixNode> m_pMatrixNode;
		bool m_AllowJump;
		bool m_AllowBoost;
        BB_INSERT_PADDING(0x2);
		Hedgehog::Math::CVector m_Offset1; // SszUpDownPole only
		Hedgehog::Math::CVector m_Offset2; // SszUpDownPole only
		float m_OffsetProp; // SszUpDownPole only, 0.0f = m_Offset1, 1.0f = m_Offset2
        BB_INSERT_PADDING(0xC);

        MsgStartHangOn // sub_116E060 & sub_116DFF0
		(
			Type in_Type,
			boost::shared_ptr<Hedgehog::Mirage::CMatrixNode> in_pMatrixNode,
			bool in_AllowJump = true,
			bool in_AllowBoost = false
		) 
		: m_Type(in_Type) 
		, m_pMatrixNode(in_pMatrixNode) 
		, m_AllowJump(in_AllowJump) 
		, m_AllowBoost(in_AllowBoost) 
		, m_Offset1(Hedgehog::Math::CVector::Identity()) 
		, m_Offset2(Hedgehog::Math::CVector::Identity()) 
		, m_OffsetProp(0.0f) 
		{
		}

        MsgStartHangOn // sub_116DF80
		(
			boost::shared_ptr<Hedgehog::Mirage::CMatrixNode> in_pMatrixNode,
			Hedgehog::Math::CVector in_Offset1,
			Hedgehog::Math::CVector in_Offset2,
			float in_OffsetProp
		) 
		: m_Type(Type::SszUpDownPole) 
		, m_pMatrixNode(in_pMatrixNode) 
		, m_AllowJump(true) 
		, m_AllowBoost(false) 
		, m_Offset1(in_Offset1) 
		, m_Offset2(in_Offset2) 
		, m_OffsetProp(in_OffsetProp) 
		{
		}
    };

    BB_ASSERT_OFFSETOF(MsgStartHangOn, m_Type, 0x10);
    BB_ASSERT_OFFSETOF(MsgStartHangOn, m_pMatrixNode, 0x14);
    BB_ASSERT_OFFSETOF(MsgStartHangOn, m_AllowJump, 0x1C);
    BB_ASSERT_OFFSETOF(MsgStartHangOn, m_AllowBoost, 0x1D);
    BB_ASSERT_OFFSETOF(MsgStartHangOn, m_Offset1, 0x20);
    BB_ASSERT_OFFSETOF(MsgStartHangOn, m_Offset2, 0x30);
    BB_ASSERT_OFFSETOF(MsgStartHangOn, m_OffsetProp, 0x40);
    BB_ASSERT_SIZEOF(MsgStartHangOn, 0x50);
}