#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgStartFadeIn : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x167F288);
		
		float m_FadeTime;
		uint32_t m_BGRA;
		float m_StartAlpha;
		uint32_t m_Field1C = 0;

        MsgStartFadeIn
		(
			float in_FadeTime,
			uint32_t in_BGRA,
			float in_StartAlpha = -1.0f
		) 
		: m_FadeTime(in_FadeTime) 
		, m_BGRA(in_BGRA) 
		, m_StartAlpha(in_StartAlpha) 
		{
		}
    };

    BB_ASSERT_OFFSETOF(MsgStartFadeIn, m_FadeTime, 0x10);
    BB_ASSERT_OFFSETOF(MsgStartFadeIn, m_BGRA, 0x14);
    BB_ASSERT_OFFSETOF(MsgStartFadeIn, m_StartAlpha, 0x18);
    BB_ASSERT_OFFSETOF(MsgStartFadeIn, m_Field1C, 0x1C);
    BB_ASSERT_SIZEOF(MsgStartFadeIn, 0x20);
}