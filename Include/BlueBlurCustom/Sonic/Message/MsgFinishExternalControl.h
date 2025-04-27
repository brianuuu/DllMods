#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgFinishExternalControl : public Hedgehog::Universe::MessageTypeSet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x01680828)

		enum class EChangeState
		{
			FALL = 0x0,
			JUMP = 0x1,
			DEAD = 0x2,
			FINISH = 0x3,
			STAND = 0x4,
			ROCKETLAUNCH = 0x5,
			ROCKETLAUNCH2 = 0x6,
			SPIKEWALK = 0x7,
			SPIKEJUMP = 0x8,
			SPIKEFALL = 0x9,
		};

		EChangeState ExitState = EChangeState::FALL;
		bool EnableHomingAttack = false;
		bool SetBallModel = false;
		bool ForceFinish = false;
		int RocketLaunchInt = 0;
		Hedgehog::Math::CVector UpVector = Hedgehog::Math::CVector::Zero();

		MsgFinishExternalControl() = default;

		MsgFinishExternalControl(EChangeState in_ChangeState)
		: ExitState(in_ChangeState)
		{}

		MsgFinishExternalControl(EChangeState in_ChangeState, bool in_EnableHomingAttack)
		: ExitState(in_ChangeState), EnableHomingAttack(in_EnableHomingAttack)
		{}

		MsgFinishExternalControl(EChangeState in_ChangeState, bool in_EnableHomingAttack, bool in_SetBallModel)
		: ExitState(in_ChangeState), EnableHomingAttack(in_EnableHomingAttack), SetBallModel(in_SetBallModel)
		{}
	};
}