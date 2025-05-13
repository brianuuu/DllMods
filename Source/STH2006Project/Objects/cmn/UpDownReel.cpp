#include "UpDownReel.h"

BB_SET_OBJECT_MAKE_HOOK(UpDownReel)
void UpDownReel::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(UpDownReel);
	applyPatches();
}

HOOK(int, __fastcall, UpDownReel_CPlayerSpeedStateHangOnCPulleyJumpBegin, 0xE45CD0, Sonic::Player::CPlayerSpeedContext::CStateSpeedBase* This)
{
	// jump voice
	SharedPtrTypeless soundHandle;
	Common::SonicContextPlayVoice(soundHandle, 3002000, 0);

	return originalUpDownReel_CPlayerSpeedStateHangOnCPulleyJumpBegin(This);
}


HOOK(void, __fastcall, UpDownReel_CPlayerSpeedStateHangOnCPulleyJumpAdvance, 0xE468B0, Sonic::Player::CPlayerSpeedContext::CStateSpeedBase* This)
{
	originalUpDownReel_CPlayerSpeedStateHangOnCPulleyJumpAdvance(This);

	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	context->m_Velocity = context->m_UpVector* context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_JumpPower);
	context->m_VelocityChanged = true;
	context->m_HorizontalOrVerticalVelocityChanged = false;
}

void UpDownReel::applyPatches()
{
	INSTALL_HOOK(UpDownReel_CPlayerSpeedStateHangOnCPulleyJumpBegin);
	INSTALL_HOOK(UpDownReel_CPlayerSpeedStateHangOnCPulleyJumpAdvance);
}

void UpDownReel::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	in_rEditParam.CreateParamFloat(&m_Data.m_HeightStart, "HeightStart");
	in_rEditParam.CreateParamFloat(&m_Data.m_HeightEnd, "HeightEnd");
	in_rEditParam.CreateParamFloat(&m_Data.m_Time, "Time");
	in_rEditParam.CreateParamBool(&m_Data.m_DefaultOn, "DefaultOn");
}

bool UpDownReel::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData("cmn_updownreel01", 0);
	m_spModelBase = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModelBase->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBase, m_pMember->m_CastShadow);

	// beam
	m_spNodeModelBeam = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModelBeam->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.05f, 0.0f));
	m_spNodeModelBeam->m_Transform.m_Matrix.scale(hh::math::CVector(1.0f, m_Data.m_HeightStart - 0.9f, 0.55f));
	m_spNodeModelBeam->NotifyChanged();
	m_spNodeModelBeam->SetParent(m_spMatrixNodeTransform.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBeamData = wrapper.GetModelData("cmn_updownreel_beam", 0);
	m_spModelBeam = boost::make_shared<hh::mr::CSingleElement>(spModelBeamData);
	m_spModelBeam->BindMatrixNode(m_spNodeModelBeam);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBeam, m_pMember->m_CastShadow);

	// handle
	m_spNodeModelHandle = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModelHandle->m_Transform.SetPosition(hh::math::CVector(0.0f, -m_Data.m_HeightStart, 0.0f));
	m_spNodeModelHandle->NotifyChanged();
	m_spNodeModelHandle->SetParent(m_spMatrixNodeTransform.get());
	boost::shared_ptr<hh::mr::CModelData> spModelHoldData = wrapper.GetModelData("cmn_updownreel02", 0);
	m_spModelHandle = boost::make_shared<hh::mr::CSingleElement>(spModelHoldData);
	m_spModelHandle->BindMatrixNode(m_spNodeModelHandle);
	Sonic::CGameObject::AddRenderable("Object", m_spModelHandle, m_pMember->m_CastShadow);

	// culling
	SetCullingRange(m_Data.m_HeightStart + 10.0f);

	m_currentHeight = m_Data.m_HeightStart;
	return true;
}

bool UpDownReel::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// event collision
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEventCollision->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.3f, 0.0f));
	m_spNodeEventCollision->NotifyChanged();
	m_spNodeEventCollision->SetParent(m_spNodeModelHandle.get());
	hk2010_2_0::hkpSphereShape* shapeEventTrigger = new hk2010_2_0::hkpSphereShape(0.5f);
	AddEventCollision("Handle", shapeEventTrigger, *(int*)0x1E0AFD8, true, m_spNodeEventCollision);

	// player
	m_spNodeModelPlayer = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModelPlayer->m_Transform.SetPosition(hh::math::CVector(-1.0f, -0.04f, 0.0f));
	m_spNodeModelPlayer->NotifyChanged();
	m_spNodeModelPlayer->SetParent(m_spNodeModelHandle.get());

	return true;
}

void UpDownReel::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder,
	Sonic::CGameDocument* in_pGameDocument,
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	if ((!m_statusLoaded && !m_Data.m_DefaultOn) || !m_isOn)
	{
		m_isOn = false;
		if (m_Data.m_HeightStart > 1.5f)
		{
			m_spNodeModelHandle->m_Transform.SetPosition(hh::math::CVector(0.0f, -1.5f, 0.0f));
			m_spNodeModelHandle->NotifyChanged();
		}
	}
}

void UpDownReel::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	float constexpr c_damping = 0.7f;

	float const targetSpeed = GetTargetSpaeed();
	bool const positive = targetSpeed > 0.0f;
	float const c_acceleration = abs(targetSpeed) * 5.0f;
	float const acceleration = c_acceleration * (positive ? 1.0f : -1.0f);

	if (m_speed != 0.0f)
	{
		m_speed += acceleration * in_rUpdateInfo.DeltaTime;
		if (abs(m_speed) > abs(targetSpeed))
		{
			m_speed = targetSpeed;
		}
	}

	float const targetHeight = m_isOn ? (m_playerID ? m_Data.m_HeightEnd : m_Data.m_HeightStart) : 1.5f;
	float currentHeight = -m_spNodeModelHandle->m_Transform.m_Position.y();
	currentHeight += m_speed * in_rUpdateInfo.DeltaTime;
	if ((positive && currentHeight > targetHeight) || (!positive && currentHeight < targetHeight))
	{
		m_loopSfx.reset();
		if (m_playerID && abs(m_speed) > c_acceleration * in_rUpdateInfo.DeltaTime)
		{
			// rebound with damping
			m_speed = -m_speed * c_damping;

			SharedPtrTypeless bounceSfx;
			Common::ObjectPlaySound(this, 200600029, bounceSfx);
		}
		else
		{
			// stops
			m_speed = 0.0f;
		}

		// overshoots, cap height
		currentHeight = targetHeight;
	}

	m_spNodeModelBeam->m_Transform = hh::mr::CTransform();
	m_spNodeModelBeam->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.05f, 0.0f));
	m_spNodeModelBeam->m_Transform.m_Matrix.scale(hh::math::CVector(1.0f, currentHeight - 0.9f, 0.55f));
	m_spNodeModelBeam->NotifyChanged();
	m_spNodeModelHandle->m_Transform.SetPosition(hh::math::CVector(0.0f, -currentHeight, 0.0f));
	m_spNodeModelHandle->NotifyChanged();
}

void UpDownReel::AddParameterBank
(
	const Hedgehog::Base::CRefPtr<Sonic::CParameterBank>& in_rParameterBank
)
{
	in_rParameterBank->AccessParameterBankBool("IsOn", &m_isOn);
	m_statusLoaded = true;
}

bool UpDownReel::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (!flag)
	{
		return Sonic::CObjectBase::ProcessMessage(message, flag);
	}

	if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
	{
		auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
		if (!m_isOn && msg.m_Event == 6)
		{
			m_isOn = true;
			if (m_playerID)
			{
				// start moving if player is already hanging on
				m_loopSfx.reset();
				Common::ObjectPlaySound(this, 200600028, m_loopSfx);
			}
			m_speed = GetTargetSpaeed();
		}
		return true;
	}

	if (message.Is<Sonic::Message::MsgHitEventCollision>())
	{
		m_playerID = message.m_SenderActorID;
		SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgStartHangOn>
			(
				Sonic::Message::MsgStartHangOn::Type::UpReel, m_spNodeModelPlayer
			)
		);

		if (m_isOn)
		{
			m_loopSfx.reset();
			Common::ObjectPlaySound(this, 200600028, m_loopSfx);
			m_speed = GetTargetSpaeed();
		}
		return true;
	}

	if (message.Is<Sonic::Message::MsgExitedHangOn>())
	{
		m_loopSfx.reset();
		m_playerID = 0u;
		m_speed = GetTargetSpaeed();
		return true;
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

float UpDownReel::GetTargetSpaeed() const
{
	if (!m_isOn)
	{
		return 0.0f;
	}
	else if (m_playerID)
	{
		return (m_Data.m_HeightEnd - m_Data.m_HeightStart) / m_Data.m_Time;
	}
	else
	{
		return (m_Data.m_HeightStart - m_Data.m_HeightEnd) * 2.0f / m_Data.m_Time;
	}
}
