#include "GadgetBike.h"

uint32_t canGetOnBikeActorID = 0u;
HOOK(bool, __fastcall, GadgetBike_GroundedStateChange, 0xE013D0, Sonic::Player::CPlayerSpeedContext* context, void* Edx, int a2)
{
	if (context->m_Grounded && !context->StateFlag(eStateFlag_OutOfControl))
	{
		if (canGetOnBikeActorID && Common::fIsButtonTapped(Sonic::EKeyState::eKeyState_Y))
		{
			context->m_pPlayer->SendMessageImm(canGetOnBikeActorID, Sonic::Message::MsgNotifyObjectEvent(6));
			return true;
		}
	}

	return originalGadgetBike_GroundedStateChange(context, Edx, a2);
}

BB_SET_OBJECT_MAKE_HOOK(GadgetBike)
void GadgetBike::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(GadgetBike);
	applyPatches();
}

void GadgetBike::applyPatches()
{
	INSTALL_HOOK(GadgetBike_GroundedStateChange);
}

void GadgetBike::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	in_rEditParam.CreateParamBool(&m_Data.m_HasGun, "HasGun");
}

bool GadgetBike::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	char const* modelName = "Gadget_Bike";
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(modelName, 0);
	m_spModelBase = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModelBase->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBase, m_pMember->m_CastShadow);

	// wheels
	boost::shared_ptr<hh::mr::CModelData> spModelWheelData = wrapper.GetModelData("Gadget_Bike_Wheel", 0);
	auto const attachNodeF = m_spModelBase->GetNode("Wheel_F");
	auto const attachNodeB = m_spModelBase->GetNode("Wheel_B");
	m_spNodeWheelF = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWheelB = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWheelF->SetParent(attachNodeF.get());
	m_spNodeWheelB->SetParent(attachNodeB.get());
	m_spModelWheelF = boost::make_shared<hh::mr::CSingleElement>(spModelWheelData);
	m_spModelWheelB = boost::make_shared<hh::mr::CSingleElement>(spModelWheelData);
	m_spModelWheelF->BindMatrixNode(m_spNodeWheelF);
	m_spModelWheelB->BindMatrixNode(m_spNodeWheelB);
	Sonic::CGameObject::AddRenderable("Object", m_spModelWheelF, m_pMember->m_CastShadow);
	Sonic::CGameObject::AddRenderable("Object", m_spModelWheelB, m_pMember->m_CastShadow);

	// Guns
	if (m_Data.m_HasGun)
	{
		auto const attachNodeGunL = m_spModelBase->GetNode("GunUnder_L");
		m_spGunL = boost::make_shared<GadgetGun>("Gadget_Bike_GunL", attachNodeGunL, m_pMember->m_CastShadow, m_ActorID);
		in_pGameDocument->AddGameObject(m_spGunL, "main", this);
		auto const attachNodeGunR = m_spModelBase->GetNode("GunUnder_R");
		m_spGunR = boost::make_shared<GadgetGun>("Gadget_Bike_GunR", attachNodeGunR, m_pMember->m_CastShadow, m_ActorID);
		m_spGunR->SetIsRight();
		in_pGameDocument->AddGameObject(m_spGunR, "main", this);
	}

	// external control
	auto const attachNode = m_spModelBase->GetNode("Charapoint");
	m_spSonicControlNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spSonicControlNode->SetParent(attachNode.get());

	SetCullingRange(0.0f);

	return true;
}

bool GadgetBike::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body
	char const* rigidBodyName = "Gadget_Bike";
	AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x1E0AFF4, m_spMatrixNodeTransform, in_spDatabase);

	// damage to object
	uint32_t const typeEnemy = *(uint32_t*)0x1E5E7E8;
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	uint64_t const bitfield = (1llu << typeEnemy) | (1llu << typeBreakable);
	uint32_t const damageID = Common::MakeCollisionID(0, bitfield);
	m_spNodeCockpit = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeCockpit->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.6f, -0.1f));
	m_spNodeCockpit->NotifyChanged();
	m_spNodeCockpit->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpBoxShape* cockpitEventTrigger = new hk2010_2_0::hkpBoxShape(1.0f, 1.2f, 2.4f);
	AddEventCollision("Attack", cockpitEventTrigger, damageID, true, m_spNodeCockpit);

	// player event collision
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpCapsuleShape* shapeEventTrigger = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(0.0f, 0.5f, 1.0f), hh::math::CVector(0.0f, 0.5f, -1.0f), 2.0f);
	AddEventCollision("Player", shapeEventTrigger, *(int*)0x1E0AFD8, true, m_spNodeEventCollision); // ColID_PlayerEvent

	// fake player collision
	hk2010_2_0::hkpCylinderShape* playerEventTrigger = new hk2010_2_0::hkpCylinderShape(hh::math::CVector(0.0f, 0.0f, -0.3f), hh::math::CVector(0.0f, 1.2f, -0.3f), 0.5f);
	AddEventCollision("FakePlayer", playerEventTrigger, *(int*)0x1E0AF90, true, m_spMatrixNodeTransform); // TypePlayer
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", false);

	return true;
}

void GadgetBike::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	m_rotation = m_spMatrixNodeTransform->m_Transform.m_Rotation;
}

void GadgetBike::KillCallback()
{
	BeginPlayerGetOff(false);

	if (canGetOnBikeActorID == m_ActorID)
	{
		canGetOnBikeActorID = 0;
	}
}

void GadgetBike::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	AdvancePlayerGetOn(in_rUpdateInfo.DeltaTime);
	AdvanceDriving(in_rUpdateInfo.DeltaTime);
	AdvanceGuns(in_rUpdateInfo.DeltaTime);
	AdvancePhysics(in_rUpdateInfo.DeltaTime);
}

bool GadgetBike::ProcessMessage
(
	Hedgehog::Universe::Message& message, bool flag
)
{
	if (!flag)
	{
		return Sonic::CObjectBase::ProcessMessage(message, flag);
	}

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
				canGetOnBikeActorID = m_ActorID;
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
		if (msg.m_Symbol == "Player" && canGetOnBikeActorID == m_ActorID)
		{
			canGetOnBikeActorID = 0;
		}

		return true;
	}

	if (message.Is<Sonic::Message::MsgExitedExternalControl>())
	{
		m_playerID = 0;
		S06HUD_API::SetGadgetMaxCount(-1);
		return true;
	}

	if (message.Is<Sonic::Message::MsgGetItemType>())
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

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

bool GadgetBike::IsValidPlayer() const
{
	return *pModernSonicContext && S06DE_API::GetModelType() == S06DE_API::ModelType::Shadow;
}

bool GadgetBike::IsDriving() const
{
	return (int)m_state >= (int)State::Driving;
}

void GadgetBike::BeginPlayerGetOn()
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

void GadgetBike::AdvancePlayerGetOn(float dt)
{
	if (m_state != State::PlayerGetOn) return;

	float constexpr timeToGetOn = 2.0f;
	m_playerGetOnData.m_time += dt;

	if (m_playerGetOnData.m_time >= timeToGetOn)
	{
		m_spSonicControlNode->m_Transform.SetPosition(hh::math::CVector::Zero());
		m_spSonicControlNode->NotifyChanged();

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

void GadgetBike::BeginPlayerGetOff(bool isAlive)
{
	if (!m_playerID) return;

	m_direction = Direction::None;
	m_state = State::Idle;

	SendMessageImm(m_playerID, Sonic::Message::MsgFinishExternalControl(Sonic::Message::MsgFinishExternalControl::EChangeState::FALL));

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

		// unload gun
		if (m_Data.m_HasGun)
		{
			SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
			SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));

			SharedPtrTypeless sfx;
			Common::ObjectPlaySound(this, 200612013, sfx);
		}
	}
	else
	{
		Common::SetPlayerVelocity(velocity);
	}

	S06HUD_API::SetGadgetMaxCount(-1);

	// TODO: sfx
	ToggleBrakeLights(false);
	//m_doubleTapTime = 0.0f;

	// out of control
	Common::SetPlayerOutOfControl(0.1f);

	// player collision
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", false);

}

void GadgetBike::BeginDriving()
{
	m_state = State::Driving;
	m_direction = Direction::None;

	// TODO: sfx
	SharedPtrTypeless sfx;

	// load gun
	if (m_Data.m_HasGun)
	{
		SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
		SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
		Common::ObjectPlaySound(this, 200612013, sfx);
	}

	// Change animation
	SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("Bike", true));

	// set HUD
	S06HUD_API::SetGadgetMaxCount(m_Data.m_HasGun ? 100 : 0);
	S06HUD_API::SetGadgetCount(m_Data.m_HasGun ? m_bullets : 0, m_Data.m_HasGun ? 100 : 0);
	S06HUD_API::SetGadgetHP(m_hp);

	// player collision
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", true);
}

float const c_bikeTiltMaxAngle = 15.0f * DEG_TO_RAD;
float const c_bikeTiltTurnRate = c_bikeTiltMaxAngle / 0.1f;
float const c_bikeWheelMaxAngle = 25.0f * DEG_TO_RAD;
float const c_bikeWheelTurnRate = c_bikeWheelMaxAngle / 0.1f;
float const c_bikeGunInterval = 0.05f; // f_Missile_Interval
float const c_bikeGunReloadTime = 3.0f; // f_Missile_RecoveryTime

void GadgetBike::AdvanceDriving(float dt)
{
	// helper function
	auto fnAccel = [&dt](float& value, float target, float accel)
	{
		if (value > target)
		{
			value = max(target, value - accel * dt);
		}
		else if (value < target)
		{
			value = min(target, value + accel * dt);
		}
	};

	if (m_state != State::Driving) return;

	Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();

	// calculate input, follows sub_F82000
	if (m_playerID)
	{
		m_input.x() = (abs(padState->LeftStickHorizontal) - 0.1f) / 0.8f;
		Common::ClampFloat(m_input.x(), 0.0f, 1.0f);
		if (padState->LeftStickHorizontal > 0.0f) m_input.x() *= -1.0f;
		m_input.y() = (abs(padState->LeftStickVertical) - 0.1f) / 0.8f;
		Common::ClampFloat(m_input.y(), 0.0f, 1.0f);
		if (padState->LeftStickVertical < 0.0f) m_input.y() *= -1.0f;
		if (padState->IsDown(Sonic::EKeyState::eKeyState_DpadLeft)) m_input.x() = 1.0f;
		if (padState->IsDown(Sonic::EKeyState::eKeyState_DpadRight)) m_input.x() = -1.0f;
		if (padState->IsDown(Sonic::EKeyState::eKeyState_DpadUp)) m_input.y() = 1.0f;
		if (padState->IsDown(Sonic::EKeyState::eKeyState_DpadDown)) m_input.y() = -1.0f;
	}

	// get off
	if (m_playerID && padState->IsTapped(Sonic::EKeyState::eKeyState_Y))
	{
		BeginPlayerGetOff(true);
		return;
	}

	// rotation
	if (m_input.x() != 0.0f)
	{
		fnAccel(m_tiltAngle, -m_input.x() * c_bikeTiltMaxAngle, c_bikeTiltTurnRate);
		fnAccel(m_wheelAngle, m_isLanded ? m_input.x() * c_bikeWheelMaxAngle : 0.0f, c_bikeWheelTurnRate);
	}
	else
	{
		fnAccel(m_tiltAngle, 0.0f, c_bikeTiltTurnRate);
		fnAccel(m_wheelAngle, 0.0f, c_bikeTiltTurnRate);
	}

	// TODO:

	// vulcan
	if (m_playerID && m_Data.m_HasGun)
	{
		m_bulletTimer = max(0.0f, m_bulletTimer - dt);
		if (m_bullets > 0 && m_bulletTimer <= 0.0f && padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger) && m_spGunR->IsReady() && m_spGunL->IsReady())
		{
			if (m_useGunL)
			{
				m_spGunL->FireBullet();
			}
			else
			{
				m_spGunR->FireBullet();
			}

			m_bullets--;
			m_bulletTimer = c_bikeGunInterval;
			m_reloadTimer = c_bikeGunReloadTime;
			S06HUD_API::SetGadgetCount(m_bullets, 100);
			m_useGunL = !m_useGunL;
		}
	}
}

void GadgetBike::AdvanceGuns(float dt)
{
	if (!m_Data.m_HasGun) return;

	// unload gun
	if (m_bullets == 0)
	{
		if (m_spGunL->CanUnload() && m_spGunR->CanUnload())
		{
			SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(0));
			SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(0));

			SharedPtrTypeless sfx;
			Common::ObjectPlaySound(this, 200612013, sfx);
		}
	}

	// reload
	if (m_reloadTimer > 0.0f)
	{
		m_reloadTimer -= dt;
		if (m_reloadTimer <= 0.0f)
		{
			SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(1));
			SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(1));
			m_bullets = 100;

			SharedPtrTypeless sfx;
			Common::ObjectPlaySound(this, 200612013, sfx);

			if (m_playerID)
			{
				S06HUD_API::SetGadgetCount(m_bullets, 100);
			}
		}
	}
}

void GadgetBike::AdvancePhysics(float dt)
{
	// front wheel
	{
		hh::math::CVector const upAxis = hh::math::CVector::UnitY();
		hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(m_wheelAngle, upAxis) * hh::math::CQuaternion::Identity();
		m_spNodeWheelF->m_Transform.SetRotation(newRotation);
		m_spNodeWheelF->NotifyChanged();
	}

	hh::math::CVector const forwardAxis = m_rotation * hh::math::CVector::UnitZ();
	hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(m_tiltAngle, forwardAxis) * m_rotation;
	m_spMatrixNodeTransform->m_Transform.SetRotation(newRotation);
	m_spMatrixNodeTransform->NotifyChanged();

	// player animation
	if (m_playerID)
	{
		Direction direction = GetCurrentDirection(m_input);
		if (m_direction != direction)
		{
			m_direction = direction;
			SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl(GetAnimationName().c_str()));
		}
	}
}

void GadgetBike::ToggleBrakeLights(bool on)
{
	// TODO:
}

void GadgetBike::TakeDamage(float amount)
{
	std::lock_guard guard(m_mutex);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612004, sfx);

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

void GadgetBike::Explode()
{
	m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, "ef_en_com_yh2_explosion", 1.0f, 1);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612007, sfx);

	Kill();
}

GadgetBike::Direction GadgetBike::GetCurrentDirection(hh::math::CVector2 input) const
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

std::string GadgetBike::GetAnimationName() const
{
	switch (m_direction)
	{
	case Direction::Left: return "BikeL";
	case Direction::Right: return "BikeR";
	case Direction::Back: return "BikeB";
	default: return "Bike";
	}
}
