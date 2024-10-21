#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgShakeCamera : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x16822E8);

		float m_ShakeSize;
		float m_ShakeTime;
		int m_ShakeCount;
		BB_INSERT_PADDING(0x4);
		Hedgehog::Math::CVector m_ShakeDirection;
		float m_ShakeAttenuation;
		BB_INSERT_PADDING(0xC);

        // the same order as sub_4F9E90
        MsgShakeCamera
        (
            float in_ShakeSize,
			float in_ShakeTime,
			int in_ShakeCount,
			Hedgehog::Math::CVector in_ShakeDirection,
			float in_ShakeAttenuation
        ) 
            : m_ShakeSize(in_ShakeSize)
            , m_ShakeTime(in_ShakeTime)
            , m_ShakeCount(in_ShakeCount)
            , m_ShakeDirection(in_ShakeDirection)
            , m_ShakeAttenuation(in_ShakeAttenuation)
        {}
    };

    BB_ASSERT_OFFSETOF(MsgShakeCamera, m_ShakeSize, 0x10);
    BB_ASSERT_OFFSETOF(MsgShakeCamera, m_ShakeTime, 0x14);
    BB_ASSERT_OFFSETOF(MsgShakeCamera, m_ShakeCount, 0x18);
    BB_ASSERT_OFFSETOF(MsgShakeCamera, m_ShakeDirection, 0x20);
    BB_ASSERT_OFFSETOF(MsgShakeCamera, m_ShakeAttenuation, 0x30);
    BB_ASSERT_SIZEOF(MsgShakeCamera, 0x40);
}