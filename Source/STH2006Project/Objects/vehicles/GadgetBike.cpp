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
	m_spNodeModel = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModel->SetParent(m_spMatrixNodeTransform.get());
	m_spModelBase = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModelBase->BindMatrixNode(m_spNodeModel);
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

	// mat-anim
	m_spEffectMotionAll = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
	m_spModelBase->BindEffect(m_spEffectMotionAll);

	FUNCTION_PTR(void, __thiscall, fpGetMaterialAnimData, 0x759720,
		hh::mot::CMotionDatabaseWrapper const& wrapper,
		boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData>&materialAnimData,
		hh::base::CSharedString const& name,
		uint32_t flag
	);

	FUNCTION_PTR(void, __thiscall, fpCreateMatAnim, 0x753910,
		Hedgehog::Motion::CSingleElementEffectMotionAll * This,
		boost::shared_ptr<hh::mr::CModelData> const& modelData,
		boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData> const& materialAnimData
	);

	hh::mot::CMotionDatabaseWrapper motWrapper(in_spDatabase.get());
	boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData> materialAnimData;
	fpGetMaterialAnimData(motWrapper, materialAnimData, "Gadget_Bike_Brake", 0);
	fpCreateMatAnim(m_spEffectMotionAll.get(), spModelBaseData, materialAnimData);

	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
	fpUpdateMotionAll(m_spEffectMotionAll.get(), 0.0f);

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
	AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x1E0AFF4, m_spNodeModel, in_spDatabase);

	// damage to object
	uint32_t const typeEnemy = *(uint32_t*)0x1E5E7E8;
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	uint64_t const bitfield = (1llu << typeEnemy) | (1llu << typeBreakable);
	uint32_t const damageID = Common::MakeCollisionID(0, bitfield);
	m_spNodeCockpit = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeCockpit->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.6f, -0.1f));
	m_spNodeCockpit->NotifyChanged();
	m_spNodeCockpit->SetParent(m_spNodeModel.get());
	hk2010_2_0::hkpBoxShape* cockpitEventTrigger = new hk2010_2_0::hkpBoxShape(1.0f, 1.2f, 2.4f);
	AddEventCollision("Attack", cockpitEventTrigger, damageID, true, m_spNodeCockpit);

	// player event collision
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEventCollision->SetParent(m_spNodeModel.get());
	hk2010_2_0::hkpCapsuleShape* shapeEventTrigger = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(0.0f, 0.5f, 1.0f), hh::math::CVector(0.0f, 0.5f, -1.0f), 2.0f);
	AddEventCollision("Player", shapeEventTrigger, *(int*)0x1E0AFD8, true, m_spNodeEventCollision); // ColID_PlayerEvent

	// fake player collision
	hk2010_2_0::hkpCylinderShape* playerEventTrigger = new hk2010_2_0::hkpCylinderShape(hh::math::CVector(0.0f, 0.0f, -0.3f), hh::math::CVector(0.0f, 1.2f, -0.3f), 0.5f);
	AddEventCollision("FakePlayer", playerEventTrigger, *(int*)0x1E0AF90, true, m_spNodeModel); // TypePlayer
	AddEventCollision("FakePlayerItem", playerEventTrigger, *(int*)0x1E0AF8C, true, m_spNodeModel); // TypePlayerItem
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", false);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayerItem", false);

	// proxy collision
	Hedgehog::Base::THolder<Sonic::CWorld> holder(m_pMember->m_pWorld.get());
	hk2010_2_0::hkpBoxShape* proxyShape = new hk2010_2_0::hkpBoxShape(0.7f, 0.1f, 1.9f);
	m_spProxy = boost::make_shared<Sonic::CCharacterProxy>(this, holder, proxyShape, hh::math::CVector::UnitY() * 1.5f, hh::math::CQuaternion::Identity(), *(int*)0x1E0AFAC);

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

	if (message.IsOfType((char*)0x1680D84)) // MsgApplyImpulse
	{
		auto* msg = (MsgApplyImpulse*)&message;
		hh::math::CVector const dir = msg->m_impulse.normalized();
		if (dir.dot(hh::math::CVector::UnitY()) <= 0.95f) // not pointing up
		{
			hh::math::CVector hDir = dir;
			hDir.y() = 0.0f;
			hDir.normalize();
			float yaw = acos(hDir.z());
			if (hDir.dot(Eigen::Vector3f::UnitX()) < 0) yaw = -yaw;
			m_rotation = hh::math::CQuaternion::FromTwoVectors(hDir.head<3>(), dir.head<3>()) * Eigen::AngleAxisf(yaw, Eigen::Vector3f::UnitY());
		}

		m_speed = msg->m_impulse.dot(m_rotation * hh::math::CVector::UnitZ());
		m_upSpeed = msg->m_impulse.dot(hh::math::CVector::UnitY());
		m_outOfControl = msg->m_outOfControl;
		m_tiltAngle = 0.0f;
		m_wheelAngle = 0.0f;
		m_isLanded = false;
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

		UnloadGun();
	}
	else
	{
		Common::SetPlayerVelocity(velocity);
	}

	// out of control
	Common::SetPlayerOutOfControl(0.1f);

	CleanUp();
}

void GadgetBike::UnloadGun()
{
	if (!m_Data.m_HasGun || !m_spGunR->IsStarted()) return;

	// unload gun
	SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
	SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612013, sfx);
}

void GadgetBike::CleanUp()
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

void GadgetBike::BeginDriving()
{
	m_state = State::Driving;
	m_direction = Direction::None;

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612016, sfx);
	Common::ObjectPlaySound(this, 200612017, m_loopSfx);

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
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayerItem", true);
}

float const c_bikeTiltMaxAngle = 15.0f * DEG_TO_RAD;
float const c_bikeTiltTurnRate = c_bikeTiltMaxAngle / 0.1f;
float const c_bikeWheelMaxAngle = 20.0f * DEG_TO_RAD;
float const c_bikeWheelTurnRate = c_bikeWheelMaxAngle / 0.1f;
float const c_bikeMaxSpeed = 40.0f;
float const c_bikeReverseSpeed = -5.0f;
float const c_bikeMinBrakeSpeed = 5.0f;
float const c_bikeAccel = 8.0f;
float const c_bikeBrake = 40.0f;
float const c_bikeDecel = 4.0f;
float const c_bikeGunInterval = 0.05f; // f_Missile_Interval
float const c_bikeGunReloadTime = 3.0f; // f_Missile_RecoveryTime
float const c_bikeGravity = 10.0f;
float const c_bikeDoubleTapTime = 0.2f;
float const c_bikeBoostDashTimeout = 1.0f;
float const c_bikeBoostDashDuration = 0.5f;
float const c_bikeBoostDashAccel = 40.0f;
float const c_bikeBoostDashMaxPitch = 20.0f * DEG_TO_RAD;
float const c_bikeBoostDashPitchTime = 0.15f;
float const c_bikeBrakeMaxPitch = -5.0f * DEG_TO_RAD;
float const c_bikeBrakePitchRate = abs(c_bikeBrakeMaxPitch) / 0.1f;

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

	m_outOfControl = max(0.0f, m_outOfControl - dt);
	if (m_state != State::Driving)
	{
		fnAccel(m_tiltAngle, 0.0f, c_bikeTiltTurnRate);
		fnAccel(m_wheelAngle, 0.0f, c_bikeTiltTurnRate);
		fnAccel(m_speed, 0.0f, c_bikeAccel);
		m_boostDashTime = max(-c_bikeBoostDashTimeout, m_boostDashTime - dt);
		return;
	}

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
	if (m_isLanded && m_input.x() != 0.0f)
	{
		fnAccel(m_tiltAngle, -m_input.x() * c_bikeTiltMaxAngle, c_bikeTiltTurnRate);
		fnAccel(m_wheelAngle, m_isLanded ? m_input.x() * c_bikeWheelMaxAngle : 0.0f, c_bikeWheelTurnRate);
	}
	else
	{
		fnAccel(m_tiltAngle, 0.0f, c_bikeTiltTurnRate);
		fnAccel(m_wheelAngle, 0.0f, c_bikeTiltTurnRate);
	}

	// acceleration
	bool shouldStopBrakeSfx = (m_speed <= c_bikeMinBrakeSpeed);
	if (m_isLanded)
	{
		if (m_boostDashTime > 0.0f)
		{
			// boost dash
			fnAccel(m_speed, c_bikeMaxSpeed, c_bikeBoostDashAccel);
			ToggleBrakeLights(false);
			shouldStopBrakeSfx = true;
		}
		else if (m_playerID && m_outOfControl == 0.0f && padState->IsDown(Sonic::EKeyState::eKeyState_A))
		{
			// forward
			fnAccel(m_speed, c_bikeMaxSpeed, m_speed < 0.0f ? c_bikeBrake : c_bikeAccel);
			ToggleBrakeLights(false);
			shouldStopBrakeSfx = true;
		}
		else if (m_playerID && m_outOfControl == 0.0f && padState->IsDown(Sonic::EKeyState::eKeyState_X))
		{
			// brake, reverse
			fnAccel(m_speed, c_bikeReverseSpeed, m_speed > 0.0f ? c_bikeBrake : c_bikeAccel);
			ToggleBrakeLights(true);

			// brake sfx
			if (m_speed > c_bikeMinBrakeSpeed && !m_brakeSfx)
			{
				Common::ObjectPlaySound(this, 200612020, m_brakeSfx);
			}
		}
		else
		{
			// natural stop
			fnAccel(m_speed, 0.0f, c_bikeAccel);
			ToggleBrakeLights(false);
			shouldStopBrakeSfx = true;
		}
	}
	else
	{
		ToggleBrakeLights(false);
		shouldStopBrakeSfx = true;
	}

	// stop brake sfx
	if (m_brakeSfx && shouldStopBrakeSfx)
	{
		m_brakeSfx.reset();
	}

	// boost dash
	m_doubleTapTime = max(0.0f, m_doubleTapTime - dt);
	m_boostDashTime = max(-c_bikeBoostDashTimeout, m_boostDashTime - dt);
	if (m_playerID && m_isLanded && m_outOfControl == 0.0f && m_boostDashTime <= -c_bikeBoostDashTimeout && padState->IsTapped(Sonic::EKeyState::eKeyState_A))
	{
		if (m_doubleTapTime > 0.0f)
		{
			SharedPtrTypeless sfx;
			Common::ObjectPlaySound(this, 200612019, sfx);

			m_pGlitterPlayer->PlayOneshot(m_spModelBase->GetNode("pBoost_L"), "ef_bike_boost", 1.0f, 1);
			m_pGlitterPlayer->PlayOneshot(m_spModelBase->GetNode("pBoost_R"), "ef_bike_boost", 1.0f, 1);

			m_doubleTapTime = 0.0f;
			m_boostDashTime = c_bikeBoostDashDuration;
		}
		else
		{
			m_doubleTapTime = c_bikeDoubleTapTime;
		}
	}

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
	hh::math::CVector const upAxis = hh::math::CVector::UnitY();
	m_speed = min(c_bikeMaxSpeed, m_speed);

	if (m_speed != 0.0f)
	{
		// vehicle yaw
		float const speedRatio = m_speed / c_bikeMaxSpeed;
		float constexpr steerScale = 12.0f;
		m_rotation = Eigen::AngleAxisf(m_wheelAngle * speedRatio * steerScale * dt, upAxis) * m_rotation;

		// wheel spin
		float constexpr wheelRatio = 0.68f;
		m_wheelSpin += (m_speed / wheelRatio) * dt;
		if (m_wheelSpin > PI_F * 2.0f) m_wheelSpin -= PI_F * 2.0f;
		if (m_wheelSpin < PI_F * -2.0f) m_wheelSpin += PI_F * 2.0f;
	}

	// wheels
	{
		hh::math::CVector const rightAxis = hh::math::CVector::UnitX();

		// back
		hh::math::CQuaternion newRotation = Eigen::AngleAxisf(m_wheelSpin, rightAxis) * hh::math::CQuaternion::Identity();
		m_spNodeWheelB->m_Transform.SetRotation(newRotation);
		m_spNodeWheelB->NotifyChanged();

		// front
		newRotation = Eigen::AngleAxisf(m_wheelAngle, upAxis) * newRotation;
		m_spNodeWheelF->m_Transform.SetRotation(newRotation);
		m_spNodeWheelF->NotifyChanged();
	}

	hh::math::CVector forwardAxis = m_rotation * hh::math::CVector::UnitZ();
	if (!m_isLanded)
	{
		// not grounded, let gravity handle y-axis
		forwardAxis.y() = 0.0f;
		forwardAxis.normalize();
	}

	// proxy collision
	hh::math::CVector newPosition = m_spMatrixNodeTransform->m_Transform.m_Position;
	if (m_speed != 0.0f)
	{
		m_spProxy->m_Position = m_spMatrixNodeTransform->m_Transform.m_Position;
		m_spProxy->SetRotation(m_rotation);
		m_spProxy->m_UpVector = m_rotation * hh::math::CVector::UnitY();
		m_spProxy->m_Velocity = forwardAxis * m_speed;
		Common::fCCharacterProxyIntegrate(m_spProxy.get(), dt);
		newPosition = m_spProxy->m_Position;
		m_speed = m_spProxy->m_Velocity.dot(forwardAxis);
	}

	// floor detection
	hh::math::CVector outPos = hh::math::CVector::Zero();
	hh::math::CVector outNormal = hh::math::CVector::UnitY();
	hh::math::CVector const testStart = m_spMatrixNodeTransform->m_Transform.m_Position + hh::math::CVector(0.0f, 0.5f, 0.0f);

	// hovering
	if (!m_isLanded)
	{
		m_upSpeed -= c_bikeGravity * dt;
		newPosition += hh::math::CVector::UnitY() * m_upSpeed * dt;

		// check landing
		if (m_upSpeed < 0.0f && Common::fRaycast(testStart, newPosition, outPos, outNormal, *(int*)0x1E0AFAC))
		{
			SharedPtrTypeless sfx;
			Common::ObjectPlaySound(this, 200612022, sfx);

			newPosition = outPos;
			m_upSpeed = 0.0f;
			m_isLanded = true;
		}

		// check ceiling
		hh::math::CVector const top = hh::math::CVector::UnitY() * 1.5f;
		if (m_upSpeed > 0.0f && Common::fRaycast(testStart + top, newPosition + top, outPos, outNormal, *(int*)0x1E0AFAC))
		{
			newPosition = outPos - top;
			m_upSpeed = 0.0f;
		}
	}
	else
	{
		// check leaving terrain
		hh::math::CVector const testEnd = m_spMatrixNodeTransform->m_Transform.m_Position + hh::math::CVector(0.0f, -0.25f, 0.0f);
		if (Common::fRaycast(testStart, testEnd, outPos, outNormal, *(int*)0x1E0AFAC))
		{
			newPosition.y() = outPos.y();

			// pitch
			float constexpr pitchRate = 10.0f;
			hh::math::CVector const upAxis = m_rotation * hh::math::CVector::UnitY();
			hh::math::CQuaternion const targetPitch = hh::math::CQuaternion::FromTwoVectors(upAxis.head<3>(), outNormal.head<3>());
			m_rotation = hh::math::CQuaternion::Identity().slerp(dt * pitchRate, targetPitch) * m_rotation;
		}
		else
		{
			m_isLanded = false;

			// left ground, separate up/forward component
			hh::math::CVector forwardHorizontal = forwardAxis;
			forwardHorizontal.y() = 0.0f;
			forwardHorizontal.normalize();
			m_speed = forwardHorizontal.dot(forwardAxis * m_speed);
			m_upSpeed = hh::math::CVector::UnitY().dot(forwardAxis * m_speed);
		}
	}

	hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(m_tiltAngle, forwardAxis) * m_rotation;
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(newRotation, newPosition);
	m_spMatrixNodeTransform->NotifyChanged();

	// pitch adjustment
	{
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

		hh::math::CVector const rightAxis = hh::math::CVector::UnitX();

		if (m_brakeSfx.get())
		{
			fnAccel(m_pitchAngle, c_bikeBrakeMaxPitch, c_bikeBrakePitchRate);
		}
		else if (m_boostDashTime > 0.0f)
		{
			float const boostDashTimeNorm = c_bikeBoostDashDuration - m_boostDashTime;
			if (boostDashTimeNorm < c_bikeBoostDashPitchTime)
			{
				m_pitchAngle = c_bikeBoostDashMaxPitch * boostDashTimeNorm / c_bikeBoostDashPitchTime;
			}
			else
			{
				m_pitchAngle = c_bikeBoostDashMaxPitch * (1.0f - (boostDashTimeNorm - c_bikeBoostDashPitchTime) / (c_bikeBoostDashDuration - c_bikeBoostDashPitchTime));
			}
		}
		else if (m_pitchAngle < 0.0f)
		{
			fnAccel(m_pitchAngle, 0.0f, c_bikeBrakePitchRate);
		}
		else
		{
			m_pitchAngle = 0.0f;
		}

		float constexpr wheelOffset = 0.85f;
		float pitchUpOffset = wheelOffset * tan(abs(m_pitchAngle));
		if (m_pitchAngle < 0.0f)
		{
			float constexpr brakeUpScale = 0.5f;
			float const wheelUpOffset = (1.0f - brakeUpScale) * pitchUpOffset * cos(abs(m_pitchAngle));
			m_spNodeWheelB->m_Transform.SetPosition((wheelOffset * -2.0f * tan(abs(m_pitchAngle)) + wheelUpOffset) * hh::math::CVector::UnitY());
			m_spNodeWheelF->m_Transform.SetPosition(wheelUpOffset * hh::math::CVector::UnitY());
			m_spNodeWheelB->NotifyChanged();
			m_spNodeWheelF->NotifyChanged();

			pitchUpOffset *= brakeUpScale;
		}
		else
		{
			m_spNodeWheelB->m_Transform.SetPosition(hh::math::CVector::Zero());
			m_spNodeWheelF->m_Transform.SetPosition(hh::math::CVector::Zero());
			m_spNodeWheelB->NotifyChanged();
			m_spNodeWheelF->NotifyChanged();
		}

		hh::math::CQuaternion const adjustRotation = Eigen::AngleAxisf(m_pitchAngle, -rightAxis) * hh::math::CQuaternion::Identity();
		m_spNodeModel->m_Transform.SetRotationAndPosition(adjustRotation, pitchUpOffset * hh::math::CVector::UnitY());
		m_spNodeModel->NotifyChanged();
	}

	// player animation
	if (m_playerID && m_state == State::Driving)
	{
		Direction direction = GetCurrentDirection(m_input);
		if (m_direction != direction)
		{
			m_direction = direction;
			SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl(GetAnimationName().c_str()));
		}
	}

	// sfx pitch
	if (m_loopSfx)
	{
		hh::math::CVector* pSoundHandle = (hh::math::CVector*)m_loopSfx.get();
		pSoundHandle[2] = newPosition;

		float value = 0.5f + (m_speed + abs(m_upSpeed)) * 0.5f / c_bikeMaxSpeed;
		Common::ClampFloat(value, 0.0f, 1.0f);

		FUNCTION_PTR(void*, __thiscall, SetAisac, 0x763D50, void* This, hh::base::CSharedString const& name, float value);
		SetAisac(m_loopSfx.get(), "gadget_speed", value);
	}

	if (m_brakeSfx)
	{
		hh::math::CVector* pSoundHandle = (hh::math::CVector*)m_brakeSfx.get();
		pSoundHandle[2] = newPosition;
	}
}

void GadgetBike::ToggleBrakeLights(bool on)
{
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

void GadgetBike::TakeDamage(float amount)
{
	std::lock_guard guard(m_mutex);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612021, sfx);

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

	if (input.x() > 0 && m_outOfControl == 0.0f)
	{
		return Direction::Left;
	}

	if (input.x() < 0 && m_outOfControl == 0.0f)
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
