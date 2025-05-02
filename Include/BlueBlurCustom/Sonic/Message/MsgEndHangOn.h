#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgEndHangOn : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x167FBB8);

		float m_OutOfControl; // -1 = default disabled
		bool m_ResumeAction; // CObjBigCrane set false to set state to Nil?
		BB_INSERT_PADDING(0x3);

        MsgEndHangOn // sub_488D00
		(
			float in_OutOfControl = -1.0f,
			bool in_ResumeAction = true
		) 
		: m_OutOfControl(in_OutOfControl) 
		, m_ResumeAction(in_ResumeAction)
		{
			*(int*)0x1E5E374 = 5;
		}
    };

    BB_ASSERT_OFFSETOF(MsgEndHangOn, m_OutOfControl, 0x10);
    BB_ASSERT_OFFSETOF(MsgEndHangOn, m_ResumeAction, 0x14);
    BB_ASSERT_SIZEOF(MsgEndHangOn, 0x18);
}