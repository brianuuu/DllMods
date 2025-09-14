#include "GadgetGlider.h"
#include "NextGenShadow.h"

GadgetGlider::GadgetGlider
(
	hh::mr::CTransform const& startTrans,
	float speed,
	bool spawnGounded
)
{
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(startTrans.m_Rotation, startTrans.m_Position);
	m_spMatrixNodeTransform->NotifyChanged();

	m_speed = speed;
	m_spawnGrounded = spawnGounded;
}

GadgetGlider::~GadgetGlider()
{
	if (m_loopSfx)
	{
		m_loopSfx.reset();
	}
}

bool GadgetGlider::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	char const* modelName = "Gadget_Glider";
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(modelName, 0);
	m_spNodeModel = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModel->SetParent(m_spMatrixNodeTransform.get());
	m_spModelBase = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModelBase->BindMatrixNode(m_spNodeModel);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBase, m_pMember->m_CastShadow);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, modelName);
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo("Loop", modelName, 1.0f, hh::anim::eMotionRepeatType_Loop));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModelBase->BindPose(m_spAnimPose);

	// states
	SetContext(this);
	AddAnimationState("Loop");
	ChangeState("Loop");

	// boosters
	boost::shared_ptr<hh::mr::CModelData> spModelBoosterData = wrapper.GetModelData("Gadget_Glider_Booster", 0);
	auto const attachNodeL = m_spModelBase->GetNode("Booster_L");
	auto const attachNodeR = m_spModelBase->GetNode("Booster_R");
	m_spNodeBoosterL = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeBoosterR = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeBoosterL->SetParent(attachNodeL.get());
	m_spNodeBoosterR->SetParent(attachNodeR.get());
	m_spModelBoosterL = boost::make_shared<hh::mr::CSingleElement>(spModelBoosterData);
	m_spModelBoosterR = boost::make_shared<hh::mr::CSingleElement>(spModelBoosterData);
	m_spModelBoosterL->BindMatrixNode(m_spNodeBoosterL);
	m_spModelBoosterR->BindMatrixNode(m_spNodeBoosterR);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBoosterL, m_pMember->m_CastShadow);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBoosterR, m_pMember->m_CastShadow);

	// Guns
	float constexpr c_gliderReloadTime = 1.0f;
	auto const attachNodeGunL = m_spModelBase->GetNode("GunUnder_L");
	m_spGunL = boost::make_shared<GadgetGunSimple>("Gadget_Glider_GunL", attachNodeGunL, m_pMember->m_CastShadow, m_ActorID, c_gliderReloadTime);
	in_pGameDocument->AddGameObject(m_spGunL, "main", this);
	auto const attachNodeGunR = m_spModelBase->GetNode("GunUnder_R");
	m_spGunR = boost::make_shared<GadgetGunSimple>("Gadget_Glider_GunR", attachNodeGunR, m_pMember->m_CastShadow, m_ActorID, c_gliderReloadTime);
	in_pGameDocument->AddGameObject(m_spGunR, "main", this);

	// explosion
	m_spNodeExplodeL = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeExplodeR = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeExplodeL->m_Transform.SetPosition(hh::math::CVector(1.7f, 0.0f, 0.0f));
	m_spNodeExplodeR->m_Transform.SetPosition(hh::math::CVector(-1.7f, 0.0f, 0.0f));
	m_spNodeExplodeL->NotifyChanged();
	m_spNodeExplodeR->NotifyChanged();
	m_spNodeExplodeL->SetParent(m_spNodeModel.get());
	m_spNodeExplodeR->SetParent(m_spNodeModel.get());

	// external control
	auto const attachNode = m_spModelBase->GetNode("Charapoint");
	m_spSonicControlNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spSonicControlNode->SetParent(attachNode.get());

	SetCullingRange(0.0f);

	return true;
}

bool GadgetGlider::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body
	char const* rigidBodyName = "Gadget_Glider";
	AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x1E0AFF4, m_spNodeModel, in_spDatabase);

	// Disable rigid body for a bit
	Common::ToggleRigidBodyCollision(m_spRigidBody.get(), false);
	m_collisionEnableTimer = 0.1f;

	// damage to object
	uint32_t const typeInsulate = *(uint32_t*)0x1E5E780;
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	uint32_t const damageID = Common::MakeCollisionID(0, (1llu << typeBreakable));
	hk2010_2_0::hkpBoxShape* bodyEventTrigger = new hk2010_2_0::hkpBoxShape(4.9f, 0.7f, 2.0f);
	AddEventCollision("Enemy", bodyEventTrigger, *(int*)0x1E0AF54, true, m_spNodeModel); // ColID_TypeEnemy
	AddEventCollision("Breakable", bodyEventTrigger, damageID, true, m_spNodeModel);
	AddEventCollision("Terrain", bodyEventTrigger, *(int*)0x1E0AFAC, true, m_spNodeModel);
	AddRigidBody(m_spRigidBodyMove, bodyEventTrigger, Common::MakeCollisionID((1llu << typeInsulate), 0), m_spNodeModel);

	m_spNodeCockpit = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeCockpit->m_Transform.SetPosition(hh::math::CVector(0.0f, -0.6f, 0.0f));
	m_spNodeCockpit->NotifyChanged();
	m_spNodeCockpit->SetParent(m_spNodeModel.get());
	hk2010_2_0::hkpBoxShape* cockpitEventTrigger = new hk2010_2_0::hkpBoxShape(0.8f, 0.5f, 1.5f);
	AddEventCollision("Enemy", cockpitEventTrigger, *(int*)0x1E0AF54, true, m_spNodeCockpit); // ColID_TypeEnemy
	AddEventCollision("Breakable", cockpitEventTrigger, damageID, true, m_spNodeCockpit);
	AddEventCollision("Terrain", cockpitEventTrigger, *(int*)0x1E0AFAC, true, m_spNodeCockpit);
	AddRigidBody(m_spRigidBodyCockpit, cockpitEventTrigger, Common::MakeCollisionID((1llu << typeInsulate), 0), m_spNodeCockpit);

	// fake player collision
	hk2010_2_0::hkpCylinderShape* playerEventTrigger = new hk2010_2_0::hkpCylinderShape(hh::math::CVector::Zero(), hh::math::CVector(0.0f, 1.0f, 0.0f), 0.5f);
	AddEventCollision("FakePlayer", playerEventTrigger, *(int*)0x1E0AF90, true, m_spSonicControlNode); // TypePlayer
	AddEventCollision("FakePlayerItem", playerEventTrigger, *(int*)0x1E0AF8C, true, m_spSonicControlNode); // TypePlayerItem
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", false);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayerItem", false);

	return true;
}

void GadgetGlider::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 80041038, 1);
	m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, "ef_glider_warp", 1.0f, 1);
}

void GadgetGlider::KillCallback()
{
	BeginPlayerGetOff();
	NextGenShadow::m_vehicleSingleton.reset();
}

float const c_gliderAccel = 10.0f;
float const c_gliderMaxSpeed = 10.0f;
float const c_gliderBoostSpeed = 21.0f;
float const c_gliderMaxPitch = 45.0f * DEG_TO_RAD;
float const c_gliderSteerRate = 2.0f;
float const c_gliderSteerToAngle = 22.5f * DEG_TO_RAD;

void GadgetGlider::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	AdvancePlayerGetOn(in_rUpdateInfo.DeltaTime);
	AdvanceFlight(in_rUpdateInfo.DeltaTime);
}

bool GadgetGlider::ProcessMessage
(
	Hedgehog::Universe::Message& message, bool flag
)
{
	if (!flag)
	{
		return Sonic::CObjectBase::ProcessMessage(message, flag);
	}

	if (message.Is<Sonic::Message::MsgRestartStage>() || message.Is<Sonic::Message::MsgStageClear>())
	{
		m_playerID = 0;
		S06HUD_API::SetGadgetMaxCount(-1);
		Kill();
		return true;
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
		
		float damage = 0.0f;
		if (msg.m_Symbol == "Enemy")
		{
			damage = 8.0f;
		}
		else if (msg.m_Symbol == "Breakable")
		{
			damage = 1.0f;
		}

		if (damage > 0.0f && msg.m_SenderActorID != m_ActorID)
		{
			TakeDamage(damage);
			SendMessage(msg.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
				(
					*(uint32_t*)0x1E0BE28, // DamageID_SonicHeavy
					m_spMatrixNodeTransform->m_Transform.m_Position,
					m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ() * m_speed * 2.0f
				)
			);
		}
		
		if (msg.m_Symbol == "Terrain")
		{
			if (msg.m_SenderActorID != m_ActorID)
			{
				bool isWall = false;
				SendMessageImm(msg.m_SenderActorID, Sonic::Message::MsgIsWall(isWall));
				if (isWall)
				{
					m_hp = 0.0f;
					Explode();
				}
				else
				{
					TakeDamage(8.0f);
				}
			}
		}
		return true;
	}

	if (message.Is<Sonic::Message::MsgExitedExternalControl>())
	{
		m_playerID = 0;
		S06HUD_API::SetGadgetMaxCount(-1);
		return true;
	}

	if (message.Is<Sonic::Message::MsgDeactivate>())
	{
		if (IsFlight())
		{
			return false;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

bool GadgetGlider::IsFlight() const
{
	return (int)m_state >= (int)State::Flight;
}

void GadgetGlider::BeginPlayerGetOn()
{
	m_state = State::PlayerGetOn;

	hh::math::CVector playerPosition;
	SendMessageImm(m_playerID, Sonic::Message::MsgGetPosition(playerPosition));

	m_playerGetOnData.m_start = m_spMatrixNodeTransform->m_Transform.m_Rotation.conjugate() * (playerPosition - m_spSonicControlNode->GetWorldMatrix().translation());
	m_spSonicControlNode->m_Transform.SetPosition(m_playerGetOnData.m_start);
	m_spSonicControlNode->NotifyChanged();

	// Jump sfx
	if (m_spawnGrounded)
	{
		SharedPtrTypeless soundHandle;
		Common::SonicContextPlaySound(soundHandle, 2002027, 1);
		Common::SonicContextPlayVoice(soundHandle, 3002000, 0);
	}

	// start external control
	auto msgStartExternalControl = Sonic::Message::MsgStartExternalControl(m_spSonicControlNode, false, false);
	msgStartExternalControl.NoDamage = true;
	msgStartExternalControl.ChangeCollisionFlags = 2;
	SendMessageImm(m_playerID, msgStartExternalControl);
	SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("JumpBall", true));
}

void GadgetGlider::AdvancePlayerGetOn(float dt)
{
	if (m_state != State::PlayerGetOn) return;

	float constexpr timeToGetOn = 1.0f;
	m_playerGetOnData.m_time += dt;

	if (m_playerGetOnData.m_time >= timeToGetOn || !m_spawnGrounded)
	{
		m_spSonicControlNode->m_Transform.SetPosition(hh::math::CVector::Zero());
		m_spSonicControlNode->NotifyChanged();

		Common::fEventTrigger(this, 1);
		BeginFlight();
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

void GadgetGlider::BeginPlayerGetOff()
{
	if (!m_playerID) return;

	SendMessageImm(m_playerID, Sonic::Message::MsgFinishExternalControl(Sonic::Message::MsgFinishExternalControl::EChangeState::FALL));
	Common::SetPlayerVelocity(m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ() * m_speed);
	S06HUD_API::SetGadgetMaxCount(-1);

	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	context->StateFlag(eStateFlag_EnableHomingAttack) = true;
	context->StateFlag(eStateFlag_EnableAirOnceAction) = true;

	// out of control
	Common::SetPlayerOutOfControl(0.1f);
}

void GadgetGlider::BeginFlight()
{
	m_state = State::Flight;

	SendMessage(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	SendMessage(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));

	if (!m_loopSfx)
	{
		Common::ObjectPlaySound(this, 200612001, m_loopSfx);
	}

	// Change animation
	SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("Glider", true));

	// burner pfx
	m_burnerLID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spNodeBoosterL, "ef_jetglider_burner", 1.0f);
	m_burnerRID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spNodeBoosterR, "ef_jetglider_burner", 1.0f);

	// set HUD
	S06HUD_API::SetGadgetMaxCount(2);
	S06HUD_API::SetGadgetCount(2, 2);
	S06HUD_API::SetGadgetHP(m_hp);
	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612005, sfx);

	// player collision
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", true);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayerItem", true);
}

void GadgetGlider::AdvanceFlight(float dt)
{
	if (m_collisionEnableTimer > 0.0f)
	{
		m_collisionEnableTimer = max(0.0f, m_collisionEnableTimer - dt);
		if (m_collisionEnableTimer == 0.0f)
		{
			Common::ToggleRigidBodyCollision(m_spRigidBody.get(), true);
		}
	}

	Common::ClampFloat(m_speed, 0.0f, c_gliderBoostSpeed);

	// counterweight animation
	m_spAnimPose->Update(dt * m_speed * 0.4f);

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

	Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
	hh::math::CVector2 input = hh::math::CVector2::Zero();

	// calculate input, follows sub_F82000
	if (m_playerID)
	{
		input.x() = (abs(padState->LeftStickHorizontal) - 0.1f) / 0.8f;
		Common::ClampFloat(input.x(), 0.0f, 1.0f);
		if (padState->LeftStickHorizontal > 0.0f) input.x() *= -1.0f;
		input.y() = (abs(padState->LeftStickVertical) - 0.1f) / 0.8f;
		Common::ClampFloat(input.y(), 0.0f, 1.0f);
		if (padState->LeftStickVertical < 0.0f) input.y() *= -1.0f;
	}

	// speed
	float currentMaxSpeed = c_gliderMaxSpeed;

	// player input
	hh::math::CQuaternion newRotation = m_spMatrixNodeTransform->m_Transform.m_Rotation;
	if (m_state == State::Flight)
	{
		// get off
		if (m_playerID && padState->IsTapped(Sonic::EKeyState::eKeyState_Y))
		{
			BeginPlayerGetOff();
			m_playerID = 0;
			Explode();
			return;
		}

		// boost max speed
		if (m_playerID && padState->IsDown(Sonic::EKeyState::eKeyState_A))
		{
			currentMaxSpeed = c_gliderBoostSpeed;
		}

		// steering x-axis
		if (m_playerID && input.x() != 0.0f)
		{
			m_steer.x() += input.x() * c_gliderSteerRate * dt;
			Common::ClampFloat(m_steer.x(), -1.0f, 1.0f);
		}
		else if (abs(m_steer.x()) < 0.2f)
		{
			fnAccel(m_steer.x(), 0.0f, c_gliderSteerRate * dt * 10.0f);
		}

		if (m_steer.x() != 0.0f)
		{
			newRotation = Eigen::AngleAxisf(m_steer.x() * c_gliderSteerToAngle * dt * 1.5f, hh::math::CVector::UnitY()) * newRotation;
		}

		// steering y-axis
		if (m_playerID && ((input.y() < 0.0f && m_pitch > -c_gliderMaxPitch) || (input.y() > 0.0f && m_pitch < c_gliderMaxPitch)))
		{
			m_steer.y() += input.y() * dt;
			Common::ClampFloat(m_steer.y(), -1.0f, 1.0f);
			m_pitch += m_steer.y() * c_gliderSteerToAngle * 0.05f;
		}
		else
		{
			fnAccel(m_steer.y(), 0.0f, c_gliderSteerRate);
		}

		// player animation
		if (m_playerID)
		{
			Direction direction = GetAnimationDirection(input);
			if (m_direction != direction)
			{
				m_direction = direction;
				SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl(GetAnimationName().c_str()));
			}
		}

		// fire missiles
		if (m_playerID)
		{
			bool const rLoaded = m_spGunR->IsLoaded();
			bool const lLoaded = m_spGunL->IsLoaded();
			S06HUD_API::SetGadgetCount(rLoaded + lLoaded, 2);
			if (m_state == State::Flight && padState->IsTapped(Sonic::EKeyState::eKeyState_RightTrigger))
			{
				if (rLoaded)
				{
					SendMessage(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(0));
				}
				else if (lLoaded)
				{
					SendMessage(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(0));
				}
			}
		}
	}

	float const xSmoothed = sin(m_steer.x() * PI_F * 0.5f);

	// roll
	m_spNodeModel->m_Transform.SetRotation(Eigen::AngleAxisf(m_pitch, -hh::math::CVector::UnitX()) * Eigen::AngleAxisf(xSmoothed* c_gliderSteerToAngle, -hh::math::CVector::UnitZ()) * hh::math::CQuaternion::Identity());
	m_spNodeModel->NotifyChanged();

	// adjust speed
	fnAccel(m_speed, currentMaxSpeed, c_gliderAccel);

	hh::math::CVector const rightAxis = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitX();
	hh::math::CVector const forwardAxis = Eigen::AngleAxisf(m_pitch, -rightAxis) * m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ();
	hh::math::CVector const newPosition = m_spMatrixNodeTransform->m_Transform.m_Position + forwardAxis * m_speed * dt;
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(newRotation, newPosition);
	m_spMatrixNodeTransform->NotifyChanged();

	hh::math::CQuaternion const boosterLRotation = Eigen::AngleAxisf(xSmoothed * c_gliderSteerToAngle * 1.2f, hh::math::CVector::UnitX()) * hh::math::CQuaternion::Identity();
	m_spNodeBoosterL->m_Transform.SetRotation(boosterLRotation);
	m_spNodeBoosterL->NotifyChanged();
	hh::math::CQuaternion const boosterRRotation = Eigen::AngleAxisf(xSmoothed * c_gliderSteerToAngle * 1.2f, -hh::math::CVector::UnitX()) * hh::math::CQuaternion::Identity();
	m_spNodeBoosterR->m_Transform.SetRotation(boosterRRotation);
	m_spNodeBoosterR->NotifyChanged();

	// update loop sfx position
	if (m_loopSfx)
	{
		hh::math::CVector* pSoundHandle = (hh::math::CVector*)m_loopSfx.get();
		pSoundHandle[2] = newPosition;
	}
}

void GadgetGlider::TakeDamage(float amount)
{
	std::lock_guard guard(m_mutex);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612004, sfx);

	m_hp -= amount;
	if (m_hp <= 50.0f && !m_brokenID)
	{
		m_brokenID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spModelBase->GetNode("pBroken"), "ef_vehicle_broken", 1.0f);
	}

	if (m_playerID && IsFlight())
	{
		S06HUD_API::SetGadgetHP(max(0.0f, m_hp));
	}

	if (m_hp <= 0.0f)
	{
		Explode();
	}
}

void GadgetGlider::Explode()
{
	m_pGlitterPlayer->PlayOneshot(m_spNodeExplodeL, "ef_en_com_yh2_explosion", 1.0f, 1);
	m_pGlitterPlayer->PlayOneshot(m_spNodeExplodeR, "ef_en_com_yh2_explosion", 1.0f, 1);
	m_pGlitterPlayer->PlayOneshot(m_spNodeCockpit, "ef_en_com_yh2_explosion", 1.0f, 1);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612007, sfx);

	Kill();
}

GadgetGlider::Direction GadgetGlider::GetAnimationDirection(hh::math::CVector2 input) const
{
	float constexpr threshold = 0.2f;
	float const absX = abs(input.x());
	float const absY = abs(input.y());

	if (absX > threshold)
	{
		return input.x() > 0.0f ? Direction::Left : Direction::Right;
	}
	
	if (absY > threshold)
	{
		if (absX > threshold && absX >= absY)
		{
			return input.x() > 0.0f ? Direction::Left : Direction::Right;
		}
	}

	return Direction::None;
}

std::string GadgetGlider::GetAnimationName() const
{
	switch (m_direction)
	{
	case Direction::Left: return "GliderL";
	case Direction::Right: return "GliderR";
	default: return "Glider";
	}
}
