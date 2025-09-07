#include "GadgetJeep.h"

uint32_t canGetOnJeepActorID = 0u;
HOOK(bool, __fastcall, GadgetJeep_GroundedStateChange, 0xE013D0, Sonic::Player::CPlayerSpeedContext* context, void* Edx, int a2)
{
	if (context->m_Grounded && !context->StateFlag(eStateFlag_OutOfControl))
	{
		if (canGetOnJeepActorID && Common::fIsButtonTapped(Sonic::EKeyState::eKeyState_Y))
		{
			context->m_pPlayer->SendMessageImm(canGetOnJeepActorID, Sonic::Message::MsgNotifyObjectEvent(6));
			return true;
		}
	}

	return originalGadgetJeep_GroundedStateChange(context, Edx, a2);
}

BB_SET_OBJECT_MAKE_HOOK(GadgetJeep)
void GadgetJeep::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(GadgetJeep);
	applyPatches();
}

void GadgetJeep::applyPatches()
{
	INSTALL_HOOK(GadgetJeep_GroundedStateChange);
}

GadgetJeep::~GadgetJeep()
{
	if (canGetOnJeepActorID == m_ActorID)
	{
		canGetOnJeepActorID = 0;
	}
}

void GadgetJeep::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	in_rEditParam.CreateParamBool(&m_Data.m_CanGetOff, "CanGetOff");
	in_rEditParam.CreateParamBool(&m_Data.m_DeadNoHP, "DeadNoHP");
}

bool GadgetJeep::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	char const* modelName = "Gadget_Jeep";
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(modelName, 0);
	m_spModelBase = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModelBase->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBase, m_pMember->m_CastShadow);

	// guards
	boost::shared_ptr<hh::mr::CModelData> spModelGuardLData = wrapper.GetModelData("Gadget_Jeep_GuardL", 0);
	boost::shared_ptr<hh::mr::CModelData> spModelGuardRData = wrapper.GetModelData("Gadget_Jeep_GuardR", 0);
	auto const attachGuardL = m_spModelBase->GetNode("Guard_F_L");
	auto const attachGuardR = m_spModelBase->GetNode("Guard_F_R");
	m_spNodeGuardL = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeGuardR = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeGuardL->SetParent(attachGuardL.get());
	m_spNodeGuardR->SetParent(attachGuardR.get());
	m_spModelGuardL = boost::make_shared<hh::mr::CSingleElement>(spModelGuardLData);
	m_spModelGuardR = boost::make_shared<hh::mr::CSingleElement>(spModelGuardRData);
	m_spModelGuardL->BindMatrixNode(m_spNodeGuardL);
	m_spModelGuardR->BindMatrixNode(m_spNodeGuardR);
	Sonic::CGameObject::AddRenderable("Object", m_spModelGuardL, m_pMember->m_CastShadow);
	Sonic::CGameObject::AddRenderable("Object", m_spModelGuardR, m_pMember->m_CastShadow);

	// wheels
	boost::shared_ptr<hh::mr::CModelData> spModelWheelLData = wrapper.GetModelData("Gadget_Jeep_WheelL", 0);
	boost::shared_ptr<hh::mr::CModelData> spModelWheelRData = wrapper.GetModelData("Gadget_Jeep_WheelR", 0);
	m_spNodeWheelFL = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWheelFR = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWheelBL = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWheelBR = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWheelFL->SetParent(m_spNodeGuardL.get());
	m_spNodeWheelFR->SetParent(m_spNodeGuardR.get());
	m_spNodeWheelBL->SetParent(m_spModelBase->GetNode("Wheel_B_L").get());
	m_spNodeWheelBR->SetParent(m_spModelBase->GetNode("Wheel_B_R").get());
	m_spModelWheelFL = boost::make_shared<hh::mr::CSingleElement>(spModelWheelLData);
	m_spModelWheelFR = boost::make_shared<hh::mr::CSingleElement>(spModelWheelRData);
	m_spModelWheelBL = boost::make_shared<hh::mr::CSingleElement>(spModelWheelLData);
	m_spModelWheelBR = boost::make_shared<hh::mr::CSingleElement>(spModelWheelRData);
	m_spModelWheelFL->BindMatrixNode(m_spNodeWheelFL);
	m_spModelWheelFR->BindMatrixNode(m_spNodeWheelFR);
	m_spModelWheelBL->BindMatrixNode(m_spNodeWheelBL);
	m_spModelWheelBR->BindMatrixNode(m_spNodeWheelBR);
	Sonic::CGameObject::AddRenderable("Object", m_spModelWheelFL, m_pMember->m_CastShadow);
	Sonic::CGameObject::AddRenderable("Object", m_spModelWheelFR, m_pMember->m_CastShadow);
	Sonic::CGameObject::AddRenderable("Object", m_spModelWheelBL, m_pMember->m_CastShadow);
	Sonic::CGameObject::AddRenderable("Object", m_spModelWheelBR, m_pMember->m_CastShadow);

	// TODO: guns

	// external control
	auto const attachNode = m_spModelBase->GetNode("Charapoint");
	m_spSonicControlNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spSonicControlNode->SetParent(attachNode.get());

	return true;
}

bool GadgetJeep::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body
	char const* rigidBodyName = "Gadget_Jeep";
	AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x1E0AFF4, m_spMatrixNodeTransform, in_spDatabase);

	// Wheel collisions
	hk2010_2_0::hkpCylinderShape* wheelLShape = new hk2010_2_0::hkpCylinderShape(hh::math::CVector::Zero(), hh::math::CVector(0.5f, 0.0f, 0.0f), 0.54f);
	hk2010_2_0::hkpCylinderShape* wheelRShape = new hk2010_2_0::hkpCylinderShape(hh::math::CVector::Zero(), hh::math::CVector(-0.5f, 0.0f, 0.0f), 0.54f);
	AddRigidBody(m_spRigidBodyWheelFL, wheelLShape, *(int*)0x1E0AFF4, m_spNodeWheelFL);
	AddRigidBody(m_spRigidBodyWheelFR, wheelRShape, *(int*)0x1E0AFF4, m_spNodeWheelFR);
	AddRigidBody(m_spRigidBodyWheelBL, wheelLShape, *(int*)0x1E0AFF4, m_spNodeWheelBL);
	AddRigidBody(m_spRigidBodyWheelBR, wheelRShape, *(int*)0x1E0AFF4, m_spNodeWheelBR);

	// damage to object
	uint32_t const typeEnemy = *(uint32_t*)0x1E5E7E8;
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	uint64_t const bitfield = (1llu << typeEnemy) | (1llu << typeBreakable);
	uint32_t const damageID = Common::MakeCollisionID(0, bitfield);
	m_spNodeCockpit = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeCockpit->m_Transform.SetPosition(hh::math::CVector(0.0f, 1.0f, -0.078f));
	m_spNodeCockpit->NotifyChanged();
	m_spNodeCockpit->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpBoxShape* cockpitEventTrigger = new hk2010_2_0::hkpBoxShape(1.4f, 1.4f, 3.2f);
	AddEventCollision("Attack", cockpitEventTrigger, damageID, true, m_spNodeCockpit);

	// player event collision
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpCapsuleShape* shapeEventTrigger = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(0.0f, 1.0f, 1.25f), hh::math::CVector(0.0f, 1.0f, -1.25f), 2.5f);
	AddEventCollision("Player", shapeEventTrigger, *(int*)0x1E0AFD8, true, m_spNodeEventCollision); // ColID_PlayerEvent

	return true;
}

void GadgetJeep::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);
}

void GadgetJeep::KillCallback()
{
	BeginPlayerGetOff(false);

	if (canGetOnJeepActorID == m_ActorID)
	{
		canGetOnJeepActorID = 0;
	}
}

void GadgetJeep::GetObjectTriggerType
(
	hh::vector<uint32_t>& in_rTriggerTypeList
)
{
	in_rTriggerTypeList.push_back(1);
}

void GadgetJeep::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	AdvancePlayerGetOn(in_rUpdateInfo.DeltaTime);
}

bool GadgetJeep::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgDamage>())
		{
			if (!SendMessageImm(message.m_SenderActorID, Sonic::Message::MsgGetPlayerType()))
			{
				TakeDamage(6.0f);
			}
			return true;
		}

		if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
		{
			auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
			switch (msg.m_Event)
			{
			case 6:
			{
				if (m_state == State::Idle)
				{
					m_playerID = message.m_SenderActorID;
					BeginPlayerGetOn();
				}
				break;
			}
			case 7:
			{
				if (m_playerID)
				{
					BeginPlayerGetOff(true);
					m_playerID = 0;

					Explode();
				}
				break;
			}
			}

			return true;
		}

		if (message.Is<Sonic::Message::MsgHitEventCollision>())
		{
			auto& msg = static_cast<Sonic::Message::MsgHitEventCollision&>(message);
			if (msg.m_Symbol == "Player")
			{
				if (!IsDriving() && IsValidPlayer())
				{
					canGetOnJeepActorID = m_ActorID;
				}
			}
			else if (msg.m_Symbol == "Attack")
			{
				if (msg.m_SenderActorID != m_ActorID)
				{
					TakeDamage(8.0f);
					SendMessage(msg.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
						(
							*(uint32_t*)0x1E0BE28, // DamageID_SonicHeavy
							m_spMatrixNodeTransform->m_Transform.m_Position,
							m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ() * m_speed * 2.0f
							)
					);
				}
			}
			return true;
		}

		if (message.Is<Sonic::Message::MsgLeaveEventCollision>())
		{
			auto& msg = static_cast<Sonic::Message::MsgLeaveEventCollision&>(message);
			if (msg.m_Symbol == "Player" && canGetOnJeepActorID == m_ActorID)
			{
				canGetOnJeepActorID = 0;
			}

			return true;
		}

		if (message.Is<Sonic::Message::MsgExitedExternalControl>())
		{
			m_playerID = 0;
			UnloadGun();
			CleanUp();
			return true;
		}

		if (message.Is<Sonic::Message::MsgGetItemType>() || message.Is<Sonic::Message::MsgTakeObject>())
		{
			if (m_playerID)
			{
				// forward message to player
				SendMessageImm(m_playerID, message);
			}
			return true;
		}

		if (message.Is<Sonic::Message::MsgDeactivate>())
		{
			if (m_started)
			{
				return false;
			}
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

bool GadgetJeep::IsValidPlayer() const
{
	return *pModernSonicContext && S06DE_API::GetModelType() == S06DE_API::ModelType::Shadow;
}

bool GadgetJeep::IsDriving() const
{
	return (int)m_state >= (int)State::Driving;
}

void GadgetJeep::BeginPlayerGetOn()
{
	m_state = State::PlayerGetOn;
	m_started = true;

	hh::math::CVector playerPosition;
	SendMessageImm(m_playerID, Sonic::Message::MsgGetPosition(playerPosition));

	m_playerGetOnData.m_start = m_spMatrixNodeTransform->m_Transform.m_Rotation.conjugate() * (playerPosition - m_spSonicControlNode->GetWorldMatrix().translation());
	m_playerGetOnData.m_time = 0.0f;
	m_spSonicControlNode->m_Transform.SetPosition(m_playerGetOnData.m_start);
	m_spSonicControlNode->NotifyChanged();

	// Jump sfx
	SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 2002027, 1);
	Common::SonicContextPlayVoice(soundHandle, 3002000, 0);

	// start external control
	auto msgStartExternalControl = Sonic::Message::MsgStartExternalControl(m_spSonicControlNode, false, false);
	msgStartExternalControl.NoDamage = true;
	msgStartExternalControl.ChangeCollisionFlags = 2;
	SendMessageImm(m_playerID, msgStartExternalControl);
	SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("JumpBall", true));
}

void GadgetJeep::AdvancePlayerGetOn(float dt)
{
	if (m_state != State::PlayerGetOn) return;

	float constexpr timeToGetOn = 2.0f;
	m_playerGetOnData.m_time += dt;

	if (m_playerGetOnData.m_time >= timeToGetOn)
	{
		m_spSonicControlNode->m_Transform.SetPosition(hh::math::CVector::Zero());
		m_spSonicControlNode->NotifyChanged();

		Common::fEventTrigger(this, 1);
		BeginDriving();
		return;
	}

	float const prop = m_playerGetOnData.m_time / timeToGetOn;
	hh::math::CVector targetPosition = m_playerGetOnData.m_start * (1.0f - prop);

	float constexpr initSpeed = 7.0f;
	float constexpr accel = initSpeed * -2.0f / timeToGetOn;
	float const yOffset = initSpeed * m_playerGetOnData.m_time + 0.5f * accel * m_playerGetOnData.m_time * m_playerGetOnData.m_time;
	targetPosition.y() += yOffset;

	m_spSonicControlNode->m_Transform.SetPosition(targetPosition);
	m_spSonicControlNode->NotifyChanged();
}

void GadgetJeep::BeginPlayerGetOff(bool isAlive)
{
	if (!m_playerID) return;

	bool const deadNoHP = m_hp <= 0.0f && m_Data.m_DeadNoHP;
	SendMessageImm(m_playerID, Sonic::Message::MsgFinishExternalControl(deadNoHP ? Sonic::Message::MsgFinishExternalControl::EChangeState::DEAD : Sonic::Message::MsgFinishExternalControl::EChangeState::FALL));

	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector velocity = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ() * m_speed;
	if (isAlive)
	{
		velocity.y() = context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_JumpPower);
		Common::SetPlayerVelocity(velocity);
		hh::math::CVector position = m_spSonicControlNode->GetWorldMatrix().translation();
		position.y() += 0.8f;
		Common::SetPlayerPosition(position);

		// Jump animation
		SharedPtrTypeless soundHandle;
		Common::SonicContextPlaySound(soundHandle, 2002027, 1);
		Common::SonicContextPlayVoice(soundHandle, 3002000, 0);
		FUNCTION_PTR(void*, __thiscall, ChangeAnimationCustomPlayback, 0xE74BF0, void* context, Hedgehog::Base::CSharedString const& name, hh::math::CVector const& change);
		ChangeAnimationCustomPlayback(context, "JumpBall", hh::math::CVector::Zero());

		UnloadGun();
	}
	else if (deadNoHP)
	{
		SendMessage(m_playerID, boost::make_shared<Sonic::Message::MsgSetRotation>(m_spMatrixNodeTransform->m_Transform.m_Rotation));
	}
	else
	{
		Common::SetPlayerVelocity(velocity);
	}

	// out of control
	Common::SetPlayerOutOfControl(0.1f);

	CleanUp();
}

void GadgetJeep::UnloadGun()
{
	// TODO:
	return;

	if (!m_spGunR->IsStarted()) return;

	// unload gun
	SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
	SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612013, sfx); // TODO:
}

void GadgetJeep::CleanUp()
{
	m_state = State::Idle;
	m_direction = Direction::None;
	S06HUD_API::SetGadgetMaxCount(-1);

	m_loopSfx.reset();
	m_brakeSfx.reset();
	ToggleBrakeLights(false);
	m_doubleTapTime = 0.0f;

	// player collision
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", false);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayerItem", false);
}

void GadgetJeep::BeginDriving()
{
	m_state = State::Driving;
	m_direction = Direction::None;

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612016, sfx); // TODO: 
	Common::ObjectPlaySound(this, 200612017, m_loopSfx); // TODO: 

	// TODO: load gun
	//SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	//SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	//Common::ObjectPlaySound(this, 200612013, sfx);

	// Change animation
	SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("Jeep", true));

	// set HUD
	S06HUD_API::SetGadgetMaxCount(2);
	S06HUD_API::SetGadgetCount(2, 2); // TODO: gun loaded?
	S06HUD_API::SetGadgetHP(m_hp);

	// player collision
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", true);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayerItem", true);
}

void GadgetJeep::AdvanceDriving(float dt)
{
}

void GadgetJeep::ToggleBrakeLights(bool on)
{
	// TODO:
	return;

	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
	if (on && !m_brakeLights)
	{
		m_brakeLights = true;
		fpUpdateMotionAll(m_spEffectMotionAll.get(), 1.0f);
	}
	else if (!on && m_brakeLights)
	{
		m_brakeLights = false;
		fpUpdateMotionAll(m_spEffectMotionAll.get(), -1.0f);
	}
}

void GadgetJeep::TakeDamage(float amount)
{
	std::lock_guard guard(m_mutex);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612021, sfx); // TODO:

	m_hp -= amount;
	if (m_hp <= 50.0f && !m_brokenID)
	{
		m_brokenID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spModelBase->GetNode("pBroken"), "ef_vehicle_broken", 1.0f);
	}

	if (m_playerID && IsDriving())
	{
		S06HUD_API::SetGadgetHP(max(0.0f, m_hp));
	}

	if (m_hp <= 0.0f)
	{
		Explode();
	}
}

void GadgetJeep::Explode()
{
	m_pGlitterPlayer->PlayOneshot(m_spNodeCockpit, "ef_en_com_yh2_explosion", 1.0f, 1);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612007, sfx);

	Kill();
}

GadgetJeep::Direction GadgetJeep::GetCurrentDirection(hh::math::CVector2 input) const
{
	if (m_speed < 0.0f)
	{
		return Direction::Back;
	}

	if (input.x() > 0)
	{
		return Direction::Left;
	}

	if (input.x() < 0)
	{
		return Direction::Right;
	}

	return Direction::None;
}

std::string GadgetJeep::GetAnimationName() const
{
	switch (m_direction)
	{
	case Direction::Left: return "JeepL";
	case Direction::Right: return "JeepR";
	case Direction::Back: return "JeepB";
	default: return "Jeep";
	}
}
