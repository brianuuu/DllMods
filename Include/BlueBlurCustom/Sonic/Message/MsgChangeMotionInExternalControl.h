#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgChangeMotionInExternalControl : public Hedgehog::Universe::MessageTypeSet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x016808B8);

		Hedgehog::Base::CSharedString AnimationName;
		enum ChangeType
		{
			SPIN = 0,
			RISINGFAN = 1,
			CUSTOM = 2
		} m_ChangeType = CUSTOM;
		bool m_Field18 = true;
		bool m_Field19 = false;
		bool m_Field1A = false;
		bool m_UseOriginalBlendSpeed = true;

		MsgChangeMotionInExternalControl(const Hedgehog::Base::CSharedString& animName) : AnimationName(animName) {}

		MsgChangeMotionInExternalControl(const Hedgehog::Base::CSharedString& animName, ChangeType type)
		: AnimationName(animName), m_ChangeType(type) {}

		MsgChangeMotionInExternalControl(const Hedgehog::Base::CSharedString& animName, ChangeType type, bool a, bool b, bool c, bool overrideBlend)
		: AnimationName(animName), m_ChangeType(type), m_Field18(a), m_Field19(b), m_Field1A(c), m_UseOriginalBlendSpeed(!overrideBlend) {}

		MsgChangeMotionInExternalControl(const Hedgehog::Base::CSharedString& animName, bool a, bool b, bool c, bool overrideBlend)
		: AnimationName(animName), m_Field18(a), m_Field19(b), m_Field1A(c), m_UseOriginalBlendSpeed(!overrideBlend) {}

		MsgChangeMotionInExternalControl(const Hedgehog::Base::CSharedString& animName, bool overrideBlend)
		: AnimationName(animName), m_UseOriginalBlendSpeed(!overrideBlend) {}
	};

}