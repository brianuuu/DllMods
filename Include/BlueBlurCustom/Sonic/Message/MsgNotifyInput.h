#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgNotifyInput : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1680BA8);

        Hedgehog::Math::CVector m_Input;
		bool m_NoInput;
		BB_INSERT_PADDING(0xF);

        MsgNotifyInput(const Hedgehog::Math::CVector& in_rInput) : m_Input(in_rInput), m_NoInput(false) {}
    };

    BB_ASSERT_OFFSETOF(MsgNotifyInput, m_Input, 0x10);
    BB_ASSERT_OFFSETOF(MsgNotifyInput, m_NoInput, 0x20);
    BB_ASSERT_SIZEOF(MsgNotifyInput, 0x30);
}