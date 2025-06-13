#include "GadgetHover.h"

#include "Objects/enemy/EnemyBullet.h"

bool GadgetHoverSuspension::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument,
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	char const* modelName = "Gadget_Hover_Suspension";
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(modelName, 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spNodeParent);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_pMember->m_CastShadow);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, modelName);
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo("Loop", modelName, 1.0f, hh::anim::eMotionRepeatType_Loop));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModel->BindPose(m_spAnimPose);

	// states
	SetContext(this);
	AddAnimationState("Loop");
	ChangeState("Loop");

	return true;
}

void GadgetHoverSuspension::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	m_spMatrixNodeTransform->m_Transform.SetPosition(m_spNodeParent->GetWorldMatrix().translation());
	m_spMatrixNodeTransform->NotifyChanged();

	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
}

bool GadgetHoverGun::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	char const* modelName = "Gadget_Hover_Gun";

	// model
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(modelName, 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spNodeParent);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_castShadow);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, modelName);
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo("Load", modelName, 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo("Unload", "Gadget_Hover_Gun_rev", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo("Fire", "Gadget_Hover_Gun_Fire", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModel->BindPose(m_spAnimPose);

	// states
	SetContext(this);
	AddAnimationState("Load");
	AddAnimationState("Unload");
	AddAnimationState("Fire");
	ChangeState("Unload");
	m_spAnimPose->Update(0.35f); // 21 frames

	// set initial transform
	UpdateTransform();

	return true;
}

bool GadgetHoverGun::ProcessMessage
(
	Hedgehog::Universe::Message& message, bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
		{
			auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
			switch (msg.m_Event)
			{
			case 0:
			{
				m_loaded = false;
				break;
			}
			case 1:
			{
				m_loaded = true;
				break;
			}
			case 6:
			{
				m_started = true;
				break;
			}
			case 7:
			{
				m_started = false;
				break;
			}
			}
			return true;
		}
	}

	return Sonic::CGameObject3D::ProcessMessage(message, flag);
}

void GadgetHoverGun::UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo)
{
	// handle animations
	hh::base::CSharedString const currentState = GetCurrentState()->GetStateName();
	bool const animationFinished = Common::IsAnimationFinished(this);
	
	if (currentState == "Unload" && m_loaded && m_started && animationFinished)
	{
		ChangeState("Load");
	}
	else if (currentState != "Unload" && (!m_loaded || !m_started))
	{
		ChangeState("Unload");
	}

	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);
	UpdateTransform();
}

bool GadgetHoverGun::IsReady() const
{
	hh::base::CSharedString const currentState = GetCurrentState()->GetStateName();
	return (currentState == "Load" && Common::IsAnimationFinished(this)) || currentState == "Fire";
}

bool GadgetHoverGun::CanUnload() const
{
	hh::base::CSharedString const currentState = GetCurrentState()->GetStateName();
	return currentState != "Unload" && Common::IsAnimationFinished(this);
}

void GadgetHoverGun::UpdateTransform()
{
	// follow attach point so sound can work
	hh::math::CMatrix const matrix = m_spNodeParent->GetWorldMatrix();
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(hh::math::CQuaternion(matrix.rotation()), matrix.translation());
	m_spMatrixNodeTransform->NotifyChanged();
}

void GadgetHoverGun::FireBullet()
{
	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612014, sfx);

	ChangeState("Fire");

	auto node = m_spModel->GetNode("MissilePoint_L");
	hh::mr::CTransform startTrans;
	startTrans.m_Rotation = m_spMatrixNodeTransform->m_Transform.m_Rotation;
	startTrans.m_Position = node->GetWorldMatrix().translation();
	m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<EnemyBullet>(m_owner, startTrans));
}

uint32_t canGetOnHoverActorID = 0u;
HOOK(bool, __fastcall, GadgetHover_GroundedStateChange, 0xE013D0, Sonic::Player::CPlayerSpeedContext* context, void* Edx, int a2)
{
	if (context->m_Grounded && !context->StateFlag(eStateFlag_OutOfControl))
	{
		if (canGetOnHoverActorID && Common::fIsButtonTapped(Sonic::EKeyState::eKeyState_Y))
		{
			context->m_pPlayer->SendMessageImm(canGetOnHoverActorID, Sonic::Message::MsgNotifyObjectEvent(6));
			return true;
		}
	}

	return originalGadgetHover_GroundedStateChange(context, Edx, a2);
}

BB_SET_OBJECT_MAKE_HOOK(GadgetHover)
void GadgetHover::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(GadgetHover);
	applyPatches();
}

void GadgetHover::applyPatches()
{
	INSTALL_HOOK(GadgetHover_GroundedStateChange);
}

void GadgetHover::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
}

bool GadgetHover::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	char const* modelName = "Gadget_Hover";
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(modelName, 0);
	m_spModelBase = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModelBase->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBase, m_pMember->m_CastShadow);

	// suspension
	auto const attachNodeSuspension = m_spModelBase->GetNode("Suspension_F");
	m_spSuspension = boost::make_shared<GadgetHoverSuspension>(attachNodeSuspension, m_pMember->m_CastShadow);
	in_pGameDocument->AddGameObject(m_spSuspension, "main", this);

	// guard
	boost::shared_ptr<hh::mr::CModelData> spModelGuardData = wrapper.GetModelData("Gadget_Hover_Guard", 0);
	auto const attachNodeL = m_spModelBase->GetNode("Guard_B_L");
	auto const attachNodeR = m_spModelBase->GetNode("Guard_B_R");
	m_spNodeGuardL = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeGuardR = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeGuardL->SetParent(attachNodeL.get());
	m_spNodeGuardR->SetParent(attachNodeR.get());
	m_spModelGuardL = boost::make_shared<hh::mr::CSingleElement>(spModelGuardData);
	m_spModelGuardR = boost::make_shared<hh::mr::CSingleElement>(spModelGuardData);
	m_spModelGuardL->BindMatrixNode(m_spNodeGuardL);
	m_spModelGuardR->BindMatrixNode(m_spNodeGuardR);
	Sonic::CGameObject::AddRenderable("Object", m_spModelGuardL, m_pMember->m_CastShadow);
	Sonic::CGameObject::AddRenderable("Object", m_spModelGuardR, m_pMember->m_CastShadow);

	// Guns
	auto const attachNodeGunL = m_spModelBase->GetNode("GunUnder_L");
	m_spGunL = boost::make_shared<GadgetHoverGun>(attachNodeGunL, m_pMember->m_CastShadow, m_ActorID);
	in_pGameDocument->AddGameObject(m_spGunL, "main", this);
	auto const attachNodeGunR = m_spModelBase->GetNode("GunUnder_R");
	m_spGunR = boost::make_shared<GadgetHoverGun>(attachNodeGunR, m_pMember->m_CastShadow, m_ActorID);
	in_pGameDocument->AddGameObject(m_spGunR, "main", this);

	// external control
	auto const attachNode = m_spModelBase->GetNode("Charapoint");
	m_spSonicControlNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spSonicControlNode->SetParent(attachNode.get());

	return true;
}

bool GadgetHover::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body
	char const* rigidBodyName = "Gadget_Hover";
	AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x1E0AFF4, m_spMatrixNodeTransform, in_spDatabase);
	
	// damage to object
	uint32_t const typeEnemy = *(uint32_t*)0x1E5E7E8;
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	uint64_t const bitfield = (1llu << typeEnemy) | (1llu << typeBreakable);
	uint32_t const damageID = Common::MakeCollisionID(0, bitfield);
	hk2010_2_0::hkpCylinderShape* bodyEventTrigger = new hk2010_2_0::hkpCylinderShape(hh::math::CVector(0.0f, 0.0f, 0.54f), hh::math::CVector(0.0f, 1.7f, 0.54f), 1.2f);
	AddEventCollision("Attack", bodyEventTrigger, damageID, true, m_spMatrixNodeTransform);
	
	m_spNodeCockpit = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeCockpit->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.85f, -0.9f));
	m_spNodeCockpit->NotifyChanged();
	m_spNodeCockpit->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpBoxShape* cockpitEventTrigger = new hk2010_2_0::hkpBoxShape(1.5f, 1.7f, 1.2f);
	AddEventCollision("Attack", cockpitEventTrigger, damageID, true, m_spNodeCockpit);
	
	// player event collision
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEventCollision->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.856, 0.117));
	m_spNodeEventCollision->NotifyChanged();
	m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpSphereShape* shapeEventTrigger = new hk2010_2_0::hkpSphereShape(2.5f);
	AddEventCollision("Player", shapeEventTrigger, *(int*)0x1E0AFD8, true, m_spNodeEventCollision); // ColID_PlayerEvent

	return true;
}

void GadgetHover::KillCallback()
{
	BeginPlayerGetOff(false);

	if (canGetOnHoverActorID == m_ActorID)
	{
		canGetOnHoverActorID = 0;
	}
}

void GadgetHover::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	AdvancePlayerGetOn(in_rUpdateInfo.DeltaTime);
	AdvanceDriving(in_rUpdateInfo.DeltaTime);
	AdvanceGaurd(in_rUpdateInfo.DeltaTime);
	AdvanceGuns(in_rUpdateInfo.DeltaTime);
	AdvancePhysics(in_rUpdateInfo.DeltaTime);
}

bool GadgetHover::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
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
				canGetOnHoverActorID = m_ActorID;
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
		if (msg.m_Symbol == "Player")
		{
			canGetOnHoverActorID = 0;
		}

		return true;
	}

	if (message.Is<Sonic::Message::MsgExitedExternalControl>())
	{
		m_playerID = 0;
		S06HUD_API::SetGadgetMaxCount(0);
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

bool GadgetHover::IsValidPlayer() const
{
	return *pModernSonicContext && S06DE_API::GetModelType() == S06DE_API::ModelType::Shadow;
}

bool GadgetHover::IsDriving() const
{
	return (int)m_state >= (int)State::Driving;
}

void GadgetHover::BeginPlayerGetOn()
{
	m_state = State::PlayerGetOn;
	m_started = true;

	hh::math::CVector playerPosition;
	SendMessageImm(m_playerID, Sonic::Message::MsgGetPosition(playerPosition));

	m_playerGetOnData.m_start = playerPosition - m_spSonicControlNode->GetWorldMatrix().translation();
	m_playerGetOnData.m_start.x() *= -1.0f;
	m_playerGetOnData.m_start.z() *= -1.0f;
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

void GadgetHover::AdvancePlayerGetOn(float dt)
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

void GadgetHover::BeginPlayerGetOff(bool isAlive)
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
		SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		S06HUD_API::SetGadgetMaxCount(0);

		SharedPtrTypeless sfx;
		Common::ObjectPlaySound(this, 200612013, sfx);
	}
	else
	{
		Common::SetPlayerVelocity(velocity);
	}

	// out of control
	Common::SetPlayerOutOfControl(0.1f);
}

void GadgetHover::BeginDriving()
{
	m_state = State::Driving;

	// load gun
	SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612013, sfx);

	// Change animation
	SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("Hover", true));

	// set HUD
	S06HUD_API::SetGadgetMaxCount(100);
	S06HUD_API::SetGadgetCount(m_bullets, 100);
	S06HUD_API::SetGadgetHP(m_hp);

	// TODO:
}

float const c_hoverTurnRate = 2.0f * PI_F / 4.5f;
float const c_hoverGuardMaxAngle = 40.0f * DEG_TO_RAD;
float const c_hoverGuardTurnRate = c_hoverGuardMaxAngle / 0.1f;
float const c_hoverMaxSpeed = 40.0f;
float const c_hoverAccel = 8.0f;
float const c_hoverBrake = 20.0f;
float const c_hoverDecel = 2.0f;
float const c_hoverGunInterval = 0.05f; // f_Missile_Interval
float const c_hoverGunReloadTime = 3.0f; // f_Missile_RecoveryTime

void GadgetHover::AdvanceDriving(float dt)
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

	if (m_state != State::Driving)
	{
		fnAccel(m_guardAngle, 0.0f, c_hoverGuardTurnRate);
		return;
	}

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
		if (padState->IsDown(Sonic::EKeyState::eKeyState_DpadLeft)) input.x() = 1.0f;
		if (padState->IsDown(Sonic::EKeyState::eKeyState_DpadRight)) input.x() = -1.0f;
		if (padState->IsDown(Sonic::EKeyState::eKeyState_DpadUp)) input.y() = 1.0f;
		if (padState->IsDown(Sonic::EKeyState::eKeyState_DpadDown)) input.y() = -1.0f;
	}

	// get off
	if (m_playerID && padState->IsTapped(Sonic::EKeyState::eKeyState_Y))
	{
		BeginPlayerGetOff(true);
		return;
	}

	// rotation
	if (input.x() != 0.0f)
	{
		hh::math::CVector const upAxis = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitY();
		hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(input.x() * c_hoverTurnRate * dt, upAxis) * m_spMatrixNodeTransform->m_Transform.m_Rotation;
		m_spMatrixNodeTransform->m_Transform.SetRotation(newRotation);
		m_spMatrixNodeTransform->NotifyChanged();

		fnAccel(m_guardAngle, -input.x() * c_hoverGuardMaxAngle, c_hoverGuardTurnRate);
	}
	else
	{
		fnAccel(m_guardAngle, 0.0f, c_hoverGuardTurnRate);
	}

	// player animation
	if (m_playerID)
	{
		Direction direction = GetCurrentDirection(input);
		if (m_direction != direction)
		{
			m_direction = direction;
			SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl(GetAnimationName().c_str()));
		}
	}

	// acceleration
	if (m_playerID && padState->IsDown(Sonic::EKeyState::eKeyState_A))
	{
		// forward
		fnAccel(m_speed, c_hoverMaxSpeed, m_speed < 0.0f ? c_hoverBrake : c_hoverAccel);
	}
	else if (m_playerID && padState->IsDown(Sonic::EKeyState::eKeyState_X))
	{
		// brake, reverse
		fnAccel(m_speed, -c_hoverMaxSpeed, m_speed > 0.0f ? c_hoverBrake : c_hoverAccel);
	}
	else
	{
		// natural stop
		fnAccel(m_speed, 0.0f, c_hoverDecel);
	}

	// vulcan
	if (m_playerID)
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
			m_bulletTimer = c_hoverGunInterval;
			m_reloadTimer = c_hoverGunReloadTime;
			S06HUD_API::SetGadgetCount(m_bullets, 100);
			m_useGunL = !m_useGunL;
		}
	}
}

void GadgetHover::AdvanceGaurd(float dt)
{
	hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(m_guardAngle, hh::math::CVector::UnitY()) * hh::math::CQuaternion::Identity();
	m_spNodeGuardL->m_Transform.SetRotation(newRotation);
	m_spNodeGuardR->m_Transform.SetRotation(newRotation);
	m_spNodeGuardL->NotifyChanged();
	m_spNodeGuardR->NotifyChanged();

	// TODO: uv-anim
}

void GadgetHover::AdvanceGuns(float dt)
{
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

void GadgetHover::AdvancePhysics(float dt)
{
	hh::math::CVector const forward = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ();
	hh::math::CVector const newPosition = m_spMatrixNodeTransform->m_Transform.m_Position + forward * m_speed * dt;
	m_spMatrixNodeTransform->m_Transform.SetPosition(newPosition);
	m_spMatrixNodeTransform->NotifyChanged();
}

void GadgetHover::TakeDamage(float amount)
{
	std::lock_guard guard(m_mutex);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612004, sfx);

	m_hp -= amount;

	if (m_playerID && IsDriving())
	{
		S06HUD_API::SetGadgetHP(max(0.0f, m_hp));
	}

	if (m_hp <= 0.0f)
	{
		Explode();
	}
}

void GadgetHover::Explode()
{
	m_pGlitterPlayer->PlayOneshot(m_spModelBase->GetNode("Suspension_F"), "ef_en_com_yh2_explosion", 1.0f, 1);
	m_pGlitterPlayer->PlayOneshot(m_spModelBase->GetNode("Charapoint"), "ef_en_com_yh2_explosion", 1.0f, 1);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612007, sfx);

	Kill();
}

GadgetHover::Direction GadgetHover::GetCurrentDirection(hh::math::CVector2 input) const
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

std::string GadgetHover::GetAnimationName() const
{
	switch (m_direction)
	{
	case Direction::Left: return "HoverL";
	case Direction::Right: return "HoverR";
	case Direction::Back: return "HoverB";
	default: return "Hover";
	}
}
