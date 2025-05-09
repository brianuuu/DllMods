#include "BallSwitch.h"

BB_SET_OBJECT_MAKE_HOOK(BallSwitch)
void BallSwitch::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(BallSwitch);
}

hh::math::CVector const c_ballSwitchAnchor = hh::math::CVector(0.0f, 0.6f, 0.0f);

BallSwitch::~BallSwitch()
{
	if (m_Data.m_TargetListOFF)
	{
		m_Data.m_TargetListOFF->Release();
	}

	if (m_Data.m_TargetListON)
	{
		m_Data.m_TargetListON->Release();
	}
}

void BallSwitch::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	char const* targetListOFF = "TargetListOFF";
	m_Data.m_TargetListOFF = Sonic::CParamTargetList::Create(targetListOFF);
	if (m_Data.m_TargetListOFF)
	{
		m_Data.m_TargetListOFF->AddRef();
	}
	in_rEditParam.CreateParamBase(m_Data.m_TargetListOFF, targetListOFF);

	char const* targetListON = "TargetListON";
	m_Data.m_TargetListON = Sonic::CParamTargetList::Create(targetListON);
	if (m_Data.m_TargetListON)
	{
		m_Data.m_TargetListON->AddRef();
	}
	in_rEditParam.CreateParamBase(m_Data.m_TargetListON, targetListON);

	in_rEditParam.CreateParamInt(&m_Data.m_EventOFF, "EventOFF");
	in_rEditParam.CreateParamInt(&m_Data.m_EventON, "EventON");
	in_rEditParam.CreateParamFloat(&m_Data.m_TimerOFF, "TimerOFF");
	in_rEditParam.CreateParamFloat(&m_Data.m_TimerON, "TimerON");
	in_rEditParam.CreateParamBool(&m_Data.m_OffBeep, "OffBeep");
	in_rEditParam.CreateParamFloat(&m_Data.m_OffTimer, "OffTimer");
	in_rEditParam.CreateParamInt(&m_Data.m_Type, "Type");
}

bool BallSwitch::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData("cmn_switch", 0);
	m_spModelBase = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModelBase->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBase, m_pMember->m_CastShadow);

	// ball
	m_spNodeModelBall = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModelBall->m_Transform.SetPosition(c_ballSwitchAnchor);
	m_spNodeModelBall->NotifyChanged();
	m_spNodeModelBall->SetParent(m_spMatrixNodeTransform.get());
	boost::shared_ptr<hh::mr::CModelData> spModelData = wrapper.GetModelData("cmn_switch_ball", 0);
	m_spModelBall = boost::make_shared<hh::mr::CSingleElement>(spModelData);
	m_spModelBall->BindMatrixNode(m_spNodeModelBall);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBall, m_pMember->m_CastShadow);

	return true;
}

bool BallSwitch::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body
	char const* rigidBodyName = "cmn_switch";
	AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x1E0AFF4, m_spMatrixNodeTransform, in_spDatabase);

	// ball event collision
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEventCollision->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.6f, 0.0f));
	m_spNodeEventCollision->NotifyChanged();
	m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpSphereShape* shapeEventTrigger = new hk2010_2_0::hkpSphereShape(0.5f);
	AddEventCollision("Ball", shapeEventTrigger, *(int*)0x1E0AF34, true, m_spNodeEventCollision);

	return true;
}

void BallSwitch::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	// count down event
	if (m_eventTimer > 0.0f)
	{
		m_eventTimer -= in_rUpdateInfo.DeltaTime;
		if (m_eventTimer <= 0.0f)
		{
			m_isOn ? SendEventOn() : SendEventOff();
		}
	}

	// count down off timer
	if (m_offTimer > 0.0f && !m_isHit)
	{
		m_offTimer -= in_rUpdateInfo.DeltaTime;
		if (m_offTimer <= 0.0f)
		{
			SwitchOff();
		}
	}

	// spring physics
	float const c_damping = m_isHit ? 0.5f : 0.975f;
	float constexpr c_springConst = 8.0f;
	float constexpr c_mass = 0.01f;
	hh::math::CVector const springForce = -c_springConst * (m_spNodeModelBall->m_Transform.m_Position - c_ballSwitchAnchor);
	m_velocity += (springForce / c_mass) * in_rUpdateInfo.DeltaTime;
	m_velocity *= c_damping;
	hh::math::CVector newPosition = m_spNodeModelBall->m_Transform.m_Position + m_velocity * in_rUpdateInfo.DeltaTime;

	float constexpr maxAnchorDist = 0.75f;
	hh::math::CVector horizontalDir = newPosition - c_ballSwitchAnchor;
	if (m_isHit)
	{
		// clamp to min player dist
		hh::math::CVector playerPosition = hh::math::CVector::Zero();
		SendMessageImm(Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID, boost::make_shared<Sonic::Message::MsgGetPosition>(&playerPosition));
		if (!playerPosition.isZero())
		{
			hh::math::CVector const ballWorldPosition = m_spMatrixNodeTransform->m_Transform.m_Position + m_spMatrixNodeTransform->m_Transform.m_Rotation * newPosition;
			hh::math::CVector horizontalDir = ballWorldPosition - playerPosition;
			horizontalDir.y() = 0.0f;

			float constexpr c_minDistFromPlayer = 0.55f;
			float const currentDistToPlayer = horizontalDir.norm();
			if (currentDistToPlayer < c_minDistFromPlayer)
			{
				newPosition += horizontalDir.normalized() * (c_minDistFromPlayer - currentDistToPlayer);
			}
		}

		// move y-axis down the further the ball is when player is touching it
		horizontalDir = newPosition - c_ballSwitchAnchor;
		horizontalDir.y();

		float constexpr maxDownDist = 0.35f;
		float const currentDistToAnchor = horizontalDir.norm();
		newPosition.y() = c_ballSwitchAnchor.y() - maxDownDist * min(maxAnchorDist, currentDistToAnchor) / maxAnchorDist;
	}

	// rotate ball model (apply pitch then yaw)
	float currentForwardDistToAnchor = horizontalDir.dot(hh::math::CVector::UnitZ());
	currentForwardDistToAnchor = currentForwardDistToAnchor < 0.0f ? max(-maxAnchorDist, currentForwardDistToAnchor) : min(maxAnchorDist, currentForwardDistToAnchor);
	float constexpr c_spinRate = PI_F; // half revolution per sec
	m_angle += c_spinRate * in_rUpdateInfo.DeltaTime;
	if (m_angle > 2.0f * PI_F)
	{
		m_angle -= 2.0f * PI_F;
	}
	hh::math::CVector const upAxis = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitY(); 
	hh::math::CVector const rightAxis = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitX();
	hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(m_angle, upAxis) * Eigen::AngleAxisf(0.5f * PI_F * currentForwardDistToAnchor / maxAnchorDist, -rightAxis);

	m_spNodeModelBall->m_Transform.SetRotationAndPosition(newRotation, newPosition);
	m_spNodeModelBall->NotifyChanged();
}

void BallSwitch::GetObjectTriggerType
(
	Hedgehog::vector<uint32_t>& in_rTriggerTypeList
)
{
	FUNCTION_PTR(int, __thiscall, fpSetUpTrigger1And2, 0x1002580, void* This, Hedgehog::vector<uint32_t>& in_rTriggerTypeList);
	fpSetUpTrigger1And2(this, in_rTriggerTypeList);
}

bool BallSwitch::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (!flag)
	{
		return Sonic::CObjectBase::ProcessMessage(message, flag);
	}

	if (message.Is<Sonic::Message::MsgGetHomingAttackPriority>())
	{
		auto& msg = static_cast<Sonic::Message::MsgGetHomingAttackPriority&>(message);
		*msg.m_pPriority = 10;
		return true;
	}
	
	if (message.Is<Sonic::Message::MsgGetHomingAttackPosition>())
	{
		auto& msg = static_cast<Sonic::Message::MsgGetHomingAttackPosition&>(message);
		*msg.m_pPosition = m_spMatrixNodeTransform->m_Transform.m_Position + m_spMatrixNodeTransform->m_Transform.m_Rotation * c_ballSwitchAnchor;
		return true;
	}
	
	if (message.Is<Sonic::Message::MsgNotifyShockWave>())
	{
		SwitchOn();
		return true;
	}

	if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
	{
		auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
		if (msg.m_Event == 6)
		{
			SwitchOn();

			// disable timer forever
			m_offTimer = -1.0f;
		}
		return true;
	}

	if (message.Is<Sonic::Message::MsgHitEventCollision>())
	{
		m_isHit = true;
		SwitchOn();

		// replicate Shadow's giant hitbox in 06
		auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
		if (context->m_Velocity.y() >= 0.0f || S06DE_API::GetModelType() == S06DE_API::ModelType::Shadow)
		{
			SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamageSuccess>
				(
					m_spMatrixNodeTransform->m_Transform.m_Position + m_spMatrixNodeTransform->m_Transform.m_Rotation * c_ballSwitchAnchor, true, true, true
				)
			);
		}
		return true;
	}

	if (message.Is<Sonic::Message::MsgLeaveEventCollision>())
	{
		m_isHit = false;
		if ((Type)m_Data.m_Type == Type::SwitchOnWhileStanding)
		{
			SwitchOff();
		}
		return true;
	}

	if (message.Is<Sonic::Message::MsgDeactivate>())
	{
		if (m_isOn)
		{
			return false;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void BallSwitch::SwitchOn()
{
	// play on sfx, pfx
	Common::ObjectCGlitterPlayerOneShot(this, "ef_switch");
	Common::ObjectPlaySound(this, 200600006, m_onOffSfx);

	// set off count down timer
	if ((Type)m_Data.m_Type == Type::SwitchOffByTimer && m_offTimer > -1.0f)
	{
		m_offTimer = m_Data.m_OffTimer;
		if (m_Data.m_OffBeep && m_offTimer > 0.0f)
		{
			m_beepingSfx.reset();
			Common::ObjectPlaySound(this, 200600008, m_beepingSfx);
		}
	}

	if (m_isOn)
	{
		// already on, but can still play effects above
		return;
	}
	else
	{
		m_isOn = true;
	}

	// switch material
	FUNCTION_PTR(void*, __thiscall, CSingleElementChangeMaterial, 0x701CC0, Hedgehog::Mirage::CSingleElement* singleElement, hh::mr::CMaterialData* from, boost::shared_ptr<hh::mr::CMaterialData>&to);
	hh::mr::CMirageDatabaseWrapper wrapper(Sonic::CGameDocument::GetInstance()->m_pMember->m_spDatabase.get());
	boost::shared_ptr<hh::mr::CMaterialData> off = wrapper.GetMaterialData("cmn_switch_off");
	boost::shared_ptr<hh::mr::CMaterialData> on = wrapper.GetMaterialData("cmn_switch_on");
	CSingleElementChangeMaterial(m_spModelBall.get(), off.get(), on);

	// count down or send event immediately
	if (m_Data.m_TimerON > 0.0f)
	{
		m_eventTimer = m_Data.m_TimerON;
	}
	else
	{
		SendEventOn();
	}
}

void BallSwitch::SwitchOff()
{
	if (m_isOn)
	{
		m_isOn = false;
	}
	else
	{
		// nothing to do if it's not on
		return;
	}

	// play off sfx
	m_beepingSfx.reset();
	Common::ObjectPlaySound(this, 200600007, m_onOffSfx);

	// revert material
	FUNCTION_PTR(bool*, __thiscall, CSingleElementResetMaterial, 0x701830, Hedgehog::Mirage::CSingleElement* singleElement, hh::mr::CMaterialData* mat);
	hh::mr::CMirageDatabaseWrapper wrapper(Sonic::CGameDocument::GetInstance()->m_pMember->m_spDatabase.get());
	boost::shared_ptr<hh::mr::CMaterialData> off = wrapper.GetMaterialData("cmn_switch_off");
	CSingleElementResetMaterial(m_spModelBall.get(), off.get());

	// count down or send event immediately
	if (m_Data.m_TimerOFF > 0.0f)
	{
		m_eventTimer = m_Data.m_TimerOFF;
	}
	else
	{
		SendEventOff();
	}
}

void BallSwitch::SendEventOn()
{
	m_eventTimer = 0.0f;
	Common::fEventTrigger(this, 1);

	for (uint32_t objectID : m_Data.m_TargetListON->m_List)
	{
		Common::fSendMessageToSetObject(this, objectID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(m_Data.m_EventON));
	}
}

void BallSwitch::SendEventOff()
{
	m_eventTimer = 0.0f;
	Common::fEventTrigger(this, 2);

	for (uint32_t objectID : m_Data.m_TargetListOFF->m_List)
	{
		Common::fSendMessageToSetObject(this, objectID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(m_Data.m_EventOFF));
	}
}
