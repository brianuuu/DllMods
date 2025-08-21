#include "CustomEvent.h"

BB_SET_OBJECT_MAKE_HOOK(CustomEvent)
void CustomEvent::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(CustomEvent);
}

CustomEvent::~CustomEvent()
{
	Revert();
}

void CustomEvent::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	// Handle loading changed values
	if (!m_chaosBoostCanLevelDown)
	{
		S06DE_API::SetChaosBoostCanLevelDown(false);
	}

	if (m_chaosBoostMatchMaxLevel)
	{
		S06DE_API::SetChaosBoostMatchMaxLevel(true);
	}

	if (m_chaosBoostMaxLevel < 3)
	{
		S06DE_API::SetChaosBoostMaxLevel(m_chaosBoostMaxLevel);
	}
}

void CustomEvent::KillCallback()
{
	Revert();
}

void CustomEvent::AddParameterBank
(
	const Hedgehog::Base::CRefPtr<Sonic::CParameterBank>& in_rParameterBank
)
{
	in_rParameterBank->AccessParameterBankBool("ChaosBoostLevelDown", &m_chaosBoostCanLevelDown);
	in_rParameterBank->AccessParameterBankBool("ChaosBoostMatchMaxLevel", &m_chaosBoostMatchMaxLevel);
	in_rParameterBank->AccessParameterBankUnsignedInt("ChaosBoostMaxLevel", &m_chaosBoostMaxLevel);
}

void CustomEvent::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{

}

bool CustomEvent::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
		{
			auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
			auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
			switch (msg.m_Event)
			{
			case 0:
			{
				// Shadow start teleport begin
				S06DE_API::ToggleStartTeleport(true);
				break;
			}
			case 1:
			{
				// Shadow start teleport end
				S06DE_API::ToggleStartTeleport(false);
				break;
			}
			case 2:
			{
				// StopPeopleObject start
				SendMessageImm(context->m_pPlayer->m_ActorID, Sonic::Message::MsgStopActivity());
				Common::SetPlayerVelocity(hh::math::CVector::Zero());
				break;
			}
			case 3:
			{
				// StopPeopleObject end
				SendMessage(context->m_pPlayer->m_ActorID, boost::make_shared<Sonic::Message::MsgReopenActivity>());
				break;
			}
			case 4:
			{
				// Shadow tutorial, stop leveling down
				m_chaosBoostCanLevelDown = false;
				S06DE_API::SetChaosBoostCanLevelDown(false);
				break;
			}
			case 5:
			{
				m_chaosBoostCanLevelDown = true;
				S06DE_API::SetChaosBoostCanLevelDown(true);
				break;
			}
			case 6:
			{
				// Disable Chaos Boost
				m_chaosBoostMaxLevel = 0;
				S06DE_API::SetChaosBoostMaxLevel(0);
				break;
			}
			case 7:
			{
				// Chaos Boost Lv.1
				m_chaosBoostMaxLevel = 1;
				S06DE_API::SetChaosBoostMaxLevel(1);
				break;
			}
			case 8:
			{
				// Chaos Boost Lv.2
				m_chaosBoostMaxLevel = 2;
				S06DE_API::SetChaosBoostMaxLevel(2);
				break;
			}
			case 9:
			{
				// Chaos Boost Lv.3
				m_chaosBoostMaxLevel = 3;
				S06DE_API::SetChaosBoostMaxLevel(3);
				break;
			}
			case 10:
			{
				// Shadow tutorial, Chaos Boost match max level
				m_chaosBoostMatchMaxLevel = true;
				S06DE_API::SetChaosBoostMatchMaxLevel(true);
				break;
			}
			case 11:
			{
				m_chaosBoostMatchMaxLevel = false;
				S06DE_API::SetChaosBoostMatchMaxLevel(false);
				break;
			}
			}
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void CustomEvent::Revert()
{
	// Handle reverting changed values
	if (!m_chaosBoostCanLevelDown)
	{
		m_chaosBoostCanLevelDown = true;
		S06DE_API::SetChaosBoostCanLevelDown(true);
	}

	if (m_chaosBoostMatchMaxLevel)
	{
		m_chaosBoostMatchMaxLevel = false;
		S06DE_API::SetChaosBoostMatchMaxLevel(false);
	}

	if (m_chaosBoostMaxLevel < 3)
	{
		m_chaosBoostMaxLevel = 3;
		S06DE_API::SetChaosBoostMaxLevel(3);
	}
}
