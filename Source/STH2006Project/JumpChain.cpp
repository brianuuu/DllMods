#include "JumpChain.h"

BB_SET_OBJECT_MAKE_HOOK(JumpChain)
void JumpChain::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(JumpChain);
}

JumpChain::~JumpChain()
{
	if (m_Data.m_TargetList)
	{
		m_Data.m_TargetList->Release();
	}
}

void JumpChain::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	char const* targetList = "TargetList";
	m_Data.m_TargetList = Sonic::CParamTargetList::Create(targetList);
	if (m_Data.m_TargetList)
	{
		m_Data.m_TargetList->AddRef();
	}
	in_rEditParam.CreateParamBase(m_Data.m_TargetList, targetList);

	in_rEditParam.CreateParamFloat(&m_Data.m_LaunchSpeed, "LaunchSpeed");
	in_rEditParam.CreateParamFloat(&m_Data.m_SquatEndSpeed, "SquatEndSpeed");
	in_rEditParam.CreateParamFloat(&m_Data.m_FailOutOfControl, "FailOutOfControl");
	in_rEditParam.CreateParamBool(&m_Data.m_AutoStart, "AutoStart");
}

bool JumpChain::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// event collision
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpSphereShape* shapeEventTrigger = new hk2010_2_0::hkpSphereShape(0.5f);
	AddEventCollision("Collision", shapeEventTrigger, *(int*)0x1E0AFD8, true, m_spNodeEventCollision);

	// control node
	m_spSonicControlNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spSonicControlNode->SetParent(Sonic::CApplicationDocument::GetInstance()->m_pMember->m_spMatrixNodeRoot.get());

	return true;
}

void JumpChain::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	float constexpr c_maxClingTime = 1.0f;
	float constexpr c_noInputTime = 0.1f;
	float constexpr c_spinTime = 0.4f;

	m_timer += in_rUpdateInfo.DeltaTime;
	if (m_state == (int)State::Cling)
	{
		ClingToTarget();
		if (m_timer > c_maxClingTime)
		{
			// push Sonic out
			hh::math::CQuaternion rotation;
			if (m_targetIndex == -1)
			{
				rotation = m_spMatrixNodeTransform->m_Transform.m_Rotation;
			}
			else
			{
				Common::fSendMessageToSetObject(this, m_Data.m_TargetList->m_List.at(m_targetIndex), boost::make_shared<Sonic::Message::MsgGetRotation>(&rotation));
			}
			hh::math::CVector const upAxis = rotation * hh::math::CVector::UnitY();

			// no input fall down
			SendMessageImm(m_playerID, boost::make_shared<Sonic::Message::MsgFinishExternalControl>(Sonic::Message::MsgFinishExternalControl::EChangeState::FALL));
			Common::SetPlayerPosition(m_spSonicControlNode->m_Transform.m_Position + upAxis * 0.5f);
			m_state = (int)State::Idle;

			// out of control
			if (m_Data.m_FailOutOfControl > 0.0f)
			{
				Common::SetPlayerOutOfControl(m_Data.m_FailOutOfControl);
			}
			return;
		}

		if (m_timer > c_noInputTime || CanAutoJump())
		{
			Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
			if (padState->IsTapped(Sonic::EKeyState::eKeyState_A) || CanAutoJump())
			{
				if (!CanAutoJump())
				{
					SharedPtrTypeless soundHandle;
					Common::SonicContextPlaySound(soundHandle, 2002027, 1);
					Common::SonicContextPlayVoice(soundHandle, 3002000, 0);
				}

				// launch to next target
				m_state = (int)State::Launch;
				m_timer = 0.0f;
				m_targetIndex++;

				SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("JumpBall"));
			}
		}
	}
	else if (m_state == (int)State::SquatEnd)
	{
		ClingToTarget();
		if (m_timer > c_maxClingTime)
		{
			// launch forward (mach speed)
			SendMessageImm(m_playerID, Sonic::Message::MsgFinishExternalControl(Sonic::Message::MsgFinishExternalControl::EChangeState::STAND));
			Common::SetPlayerVelocity(m_spSonicControlNode->m_Transform.m_Rotation * hh::math::CVector::UnitZ() * m_Data.m_SquatEndSpeed);
			m_state = (int)State::Idle;
			return;
		}
	}
	
	// no else if, we want to launch Sonic immediately after input
	if (m_state == (int)State::Launch || m_state == (int)State::LaunchFar)
	{
		if (m_state == (int)State::Launch && m_timer > c_spinTime)
		{
			SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("FallFast"));
			m_state = (int)State::LaunchFar;
		}

		hh::math::CVector targetPosition;
		Common::fSendMessageToSetObject(this, m_Data.m_TargetList->m_List.at(m_targetIndex), boost::make_shared<Sonic::Message::MsgGetPosition>(&targetPosition));

		hh::math::CVector const dir = targetPosition - m_spSonicControlNode->m_Transform.m_Position;
		if (dir.norm() <= m_Data.m_LaunchSpeed * in_rUpdateInfo.DeltaTime)
		{
			if (m_targetIndex < m_Data.m_TargetList->m_List.size() - 1 // all middle targets
			 || (m_Data.m_SquatEndSpeed > 0.0f && m_targetIndex == m_Data.m_TargetList->m_List.size() - 1)) // mach speed cling to last target
			{
				// cling to next target
				StartCling();
			}
			else
			{
				// finished
				SendMessageImm(m_playerID, Sonic::Message::MsgFinishExternalControl(Sonic::Message::MsgFinishExternalControl::EChangeState::FALL));
				m_state = (int)State::Idle;
				return;
			}
		}
		else
		{
			// moving to target
			hh::math::CVector dirXZ = dir; dirXZ.y() = 0.0f;
			hh::math::CQuaternion const rotYaw = Hedgehog::Math::CQuaternion::FromTwoVectors(Hedgehog::Math::CVector::UnitZ(), dirXZ.head<3>());
			hh::math::CVector const position = m_spSonicControlNode->m_Transform.m_Position + dir.normalized() * m_Data.m_LaunchSpeed * in_rUpdateInfo.DeltaTime;
			m_spSonicControlNode->m_Transform.SetRotationAndPosition(rotYaw, position);
			m_spSonicControlNode->NotifyChanged();
		}
	}
}

bool JumpChain::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (!flag)
	{
		return Sonic::CObjectBase::ProcessMessage(message, flag);
	}

	if (message.Is<Sonic::Message::MsgHitEventCollision>())
	{
		if (m_state == (int)State::Idle && m_Data.m_LaunchSpeed > 0.0f && !m_Data.m_TargetList->m_List.empty())
		{
			m_playerID = message.m_SenderActorID;
			m_targetIndex = -1;
			StartCling();
		}
		
		return true;
	}

	if (message.Is<Sonic::Message::MsgExitedExternalControl>())
	{
		// got interrupted (damage etc.)
		m_state = (int)State::Idle;
		return true;
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void JumpChain::StartCling()
{
	m_state = (int)State::Cling;
	m_timer = 0.0f;

	ClingToTarget();
	if (m_targetIndex == -1)
	{
		// start external control
		SendMessageImm(m_playerID, Sonic::Message::MsgStartExternalControl(m_spSonicControlNode, false, false));
	}
	else if (m_Data.m_SquatEndSpeed > 0.0f && m_targetIndex == m_Data.m_TargetList->m_List.size() - 1)
	{
		m_state = (int)State::SquatEnd;
	}

	// change animation
	SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("Squat"));
}

void JumpChain::ClingToTarget()
{
	if (m_targetIndex == -1)
	{
		// cling to itself
		m_spSonicControlNode->m_Transform = m_spMatrixNodeTransform->m_Transform;
	}
	else
	{
		// cling to current target
		hh::math::CQuaternion targetRotation;
		Common::fSendMessageToSetObject(this, m_Data.m_TargetList->m_List.at(m_targetIndex), boost::make_shared<Sonic::Message::MsgGetRotation>(&targetRotation));
		hh::math::CVector targetPosition;
		Common::fSendMessageToSetObject(this, m_Data.m_TargetList->m_List.at(m_targetIndex), boost::make_shared<Sonic::Message::MsgGetPosition>(&targetPosition));
		m_spSonicControlNode->m_Transform.SetRotationAndPosition(targetRotation, targetPosition);
	}
	m_spSonicControlNode->NotifyChanged();
}

bool JumpChain::CanAutoJump() const
{
	return m_Data.m_AutoStart && m_targetIndex == -1;
}
