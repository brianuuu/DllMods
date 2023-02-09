#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgEndQuickStepSign : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1680634);

        bool m_HadInput;
		bool m_IsRight;
        BB_INSERT_PADDING(0x2);

        // the same order as sub_4F9E90
        MsgEndQuickStepSign
        (
            bool in_HadInput,
            bool in_IsRight
        ) 
            : m_HadInput(in_HadInput)
            , m_IsRight(in_IsRight)
        {}
    };

    BB_ASSERT_OFFSETOF(MsgEndQuickStepSign, m_HadInput, 0x10);
    BB_ASSERT_OFFSETOF(MsgEndQuickStepSign, m_IsRight, 0x11);
    BB_ASSERT_SIZEOF(MsgEndQuickStepSign, 0x14);
}