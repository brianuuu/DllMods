#include "GadgetHover.h"

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

	SetCullingRange(0.0f);

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
	m_spGunL = boost::make_shared<GadgetGun>("Gadget_Hover_Gun", attachNodeGunL, m_pMember->m_CastShadow, m_ActorID);
	in_pGameDocument->AddGameObject(m_spGunL, "main", this);
	auto const attachNodeGunR = m_spModelBase->GetNode("GunUnder_R");
	m_spGunR = boost::make_shared<GadgetGun>("Gadget_Hover_Gun", attachNodeGunR, m_pMember->m_CastShadow, m_ActorID);
	in_pGameDocument->AddGameObject(m_spGunR, "main", this);

	// external control
	auto const attachNode = m_spModelBase->GetNode("Charapoint");
	m_spSonicControlNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spSonicControlNode->SetParent(attachNode.get());

	// mat-anim
	m_spEffectMotionAll = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
	m_spModelBase->BindEffect(m_spEffectMotionAll);
	m_spEffectMotionGuardL = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
	m_spModelGuardL->BindEffect(m_spEffectMotionGuardL);
	m_spEffectMotionGuardR = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
	m_spModelGuardR->BindEffect(m_spEffectMotionGuardR);

	FUNCTION_PTR(void, __thiscall, fpGetMaterialAnimData, 0x759720,
		hh::mot::CMotionDatabaseWrapper const& wrapper,
		boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData>& materialAnimData,
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
	fpGetMaterialAnimData(motWrapper, materialAnimData, "Gadget_Hover_Reverse", 0);
	fpCreateMatAnim(m_spEffectMotionAll.get(), spModelBaseData, materialAnimData);
	fpGetMaterialAnimData(motWrapper, materialAnimData, "Gadget_Hover_Accel", 0);
	fpCreateMatAnim(m_spEffectMotionGuardL.get(), spModelGuardData, materialAnimData);
	fpCreateMatAnim(m_spEffectMotionGuardR.get(), spModelGuardData, materialAnimData);

	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
	fpUpdateMotionAll(m_spEffectMotionAll.get(), 0.0f);
	fpUpdateMotionAll(m_spEffectMotionGuardL.get(), 0.25f);
	fpUpdateMotionAll(m_spEffectMotionGuardR.get(), 0.25f);

	SetCullingRange(0.0f);

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

	// fake player collision
	hk2010_2_0::hkpCylinderShape* playerEventTrigger = new hk2010_2_0::hkpCylinderShape(hh::math::CVector(0.0f, 0.85f, -0.63f), hh::math::CVector(0.0f, 1.85f, -0.63f), 0.5f);
	AddEventCollision("FakePlayer", playerEventTrigger, *(int*)0x1E0AF90, true, m_spMatrixNodeTransform); // TypePlayer
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", false);

	// land collision
	uint32_t const typeTerrain = *(uint32_t*)0x1E5E754;
	uint32_t const typeWater = *(uint32_t*)0x1E5E7C8;
	uint64_t const bitfieldLand = (1llu << typeTerrain) | (1llu << typeWater);
	m_landCollisionID = Common::MakeCollisionID(0, bitfieldLand);

	// proxy collision
	Hedgehog::Base::THolder<Sonic::CWorld> holder(m_pMember->m_pWorld.get());
	hk2010_2_0::hkpCylinderShape* proxyShape = new hk2010_2_0::hkpCylinderShape(hh::math::CVector(0.0f, 2.0f, 0.0f), hh::math::CVector(0.0f, 2.1f, 0.0f), 1.5f);
	m_spProxy = boost::make_shared<Sonic::CCharacterProxy>(this, holder, proxyShape, hh::math::CVector::Zero(), hh::math::CQuaternion::Identity(), *(int*)0x1E0AFAC);

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
		if (msg.m_Symbol == "Player" && canGetOnHoverActorID == m_ActorID)
		{
			canGetOnHoverActorID = 0;
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

void GadgetHover::UnloadGun()
{
	if (!m_spGunR->IsReady()) return;

	// unload gun
	SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
	SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612013, sfx);
}

void GadgetHover::CleanUp()
{
	m_state = State::Idle;
	m_direction = Direction::None;
	S06HUD_API::SetGadgetMaxCount(-1);

	m_loopSfx.reset();
	m_brakeSfx.reset();
	m_guardLights = false;
	ToggleBrakeLights(false);
	m_doubleTapTime = 0.0f;

	// player collision
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", false);

	// pfx
	if (m_suspensionID)
	{
		m_pGlitterPlayer->StopByID(m_suspensionID, false);
		m_suspensionID = 0;
	}
}

void GadgetHover::BeginDriving()
{
	m_state = State::Driving;
	m_direction = Direction::None;

	// load gun
	SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612013, sfx);
	Common::ObjectPlaySound(this, 200612008, sfx);
	Common::ObjectPlaySound(this, 200612009, m_loopSfx);

	// Change animation
	SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("Hover", true));

	// set HUD
	S06HUD_API::SetGadgetMaxCount(100);
	S06HUD_API::SetGadgetCount(m_bullets, 100);
	S06HUD_API::SetGadgetHP(m_hp);

	// player collision
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", true);

	// pfx
	m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, "ef_hover_start", 1.0f, 1);
	if (!m_suspensionID)
	{
		m_suspensionID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spMatrixNodeTransform, "ef_hover_suspension", 1.0f);
	}
}

float const c_hoverTurnRate = 2.0f * PI_F / 4.5f;
float const c_hoverGuardMaxAngle = 40.0f * DEG_TO_RAD;
float const c_hoverGuardTurnRate = c_hoverGuardMaxAngle / 0.1f;
float const c_hoverMaxSpeed = 40.0f;
float const c_hoverMaxSpeedSteering = 20.0f;
float const c_hoverMinBrakeSpeed = 5.0f;
float const c_hoverAccel = 8.0f;
float const c_hoverBrake = 20.0f;
float const c_hoverDecel = 2.0f;
float const c_hoverGunInterval = 0.05f; // f_Missile_Interval
float const c_hoverGunReloadTime = 3.0f; // f_Missile_RecoveryTime
float const c_hoverDoubleTapTime = 0.2f;
float const c_hoverJumpGuardTime = 0.25f;
float const c_hoverJumpAccel = 18.0f;
float const c_hoverJumpAccelTime = 0.5f;
float const c_hoverJumpHoldSpeed = -0.5f;
float const c_hoverGravity = 10.0f;

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
		fnAccel(m_speed, 0.0f, c_hoverDecel);
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
	if (m_input.x() != 0.0f)
	{
		hh::math::CVector const upAxis = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitY();
		hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(m_input.x() * c_hoverTurnRate * dt, upAxis) * m_spMatrixNodeTransform->m_Transform.m_Rotation;
		m_spMatrixNodeTransform->m_Transform.SetRotation(newRotation);
		m_spMatrixNodeTransform->NotifyChanged();

		fnAccel(m_guardAngle, -m_input.x() * c_hoverGuardMaxAngle, c_hoverGuardTurnRate);
	}
	else
	{
		fnAccel(m_guardAngle, 0.0f, c_hoverGuardTurnRate);
	}

	// max speed
	float const currentMaxSpeed = m_input.x() == 0.0f ? c_hoverMaxSpeed : c_hoverMaxSpeedSteering;

	// acceleration
	bool shouldStopBrakeSfx = (m_speed <= c_hoverMinBrakeSpeed);
	if (m_playerID && m_isLanded && padState->IsDown(Sonic::EKeyState::eKeyState_A))
	{
		// forward
		fnAccel(m_speed, currentMaxSpeed, m_speed < 0.0f ? c_hoverBrake : c_hoverAccel);
		m_guardLights = true;
		ToggleBrakeLights(false);
		shouldStopBrakeSfx = true;
	}
	else if (m_playerID && m_isLanded && padState->IsDown(Sonic::EKeyState::eKeyState_X))
	{
		// brake, reverse
		fnAccel(m_speed, -currentMaxSpeed, m_speed > 0.0f ? c_hoverBrake : c_hoverAccel);
		m_guardLights = false;
		ToggleBrakeLights(true);

		// brake sfx
		if (m_speed > c_hoverMinBrakeSpeed && !m_brakeSfx)
		{
			Common::ObjectPlaySound(this, 200612011, m_brakeSfx);
		}
	}
	else
	{
		// natural stop
		fnAccel(m_speed, 0.0f, c_hoverDecel);
		m_guardLights = !m_isLanded && padState->IsDown(Sonic::EKeyState::eKeyState_A); // floating
		ToggleBrakeLights(false);
		shouldStopBrakeSfx = true;
	}

	// stop brake sfx
	if (m_brakeSfx && shouldStopBrakeSfx)
	{
		m_brakeSfx.reset();
	}

	// jump
	m_doubleTapTime = max(0.0f, m_doubleTapTime - dt);
	if (m_playerID && m_isLanded && padState->IsTapped(Sonic::EKeyState::eKeyState_A))
	{
		if (m_doubleTapTime > 0.0f)
		{
			SharedPtrTypeless sfx;
			Common::ObjectPlaySound(this, 200612010, sfx);

			m_jumpGuardTime = c_hoverJumpGuardTime;
			m_jumpAccelTime = c_hoverJumpAccelTime;
		}
		else
		{
			m_doubleTapTime = c_hoverDoubleTapTime;
		}
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

	// sfx pitch
	if (m_loopSfx)
	{
		float value = 0.5f + (m_speed + abs(m_upSpeed)) * 0.5f / c_hoverMaxSpeed;
		Common::ClampFloat(value, 0.0f, 1.0f);

		FUNCTION_PTR(void*, __thiscall, SetAisac, 0x763D50, void* This, hh::base::CSharedString const& name, float value);
		SetAisac(m_loopSfx.get(), "gadget_speed", value);
	}
}

void GadgetHover::AdvanceGaurd(float dt)
{
	hh::math::CQuaternion newRotation = Eigen::AngleAxisf(m_guardAngle, hh::math::CVector::UnitY()) * hh::math::CQuaternion::Identity();
	if (m_jumpGuardTime > 0.0f)
	{
		float pitch = 0.0f;
		float constexpr startDownTime = 5.0f / 60.0f;
		float constexpr returnDownTime = 10.0f / 60.0f;
		if (m_jumpGuardTime > returnDownTime)
		{
			pitch = (1.0f - ((m_jumpGuardTime - returnDownTime) / startDownTime)) * PI_F * 0.5f;
		}
		else
		{
			pitch = (m_jumpGuardTime / returnDownTime) * PI_F * 0.5f;
		}

		newRotation = Eigen::AngleAxisf(pitch, -hh::math::CVector::UnitX()) * newRotation;
		m_jumpGuardTime = max(0.0f, m_jumpGuardTime - dt);
	}

	m_spNodeGuardL->m_Transform.SetRotation(newRotation);
	m_spNodeGuardR->m_Transform.SetRotation(newRotation);
	m_spNodeGuardL->NotifyChanged();
	m_spNodeGuardR->NotifyChanged();

	// mat-anim
	float constexpr guardLightMaxTime = 5.0f / 60.0f;
	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
	if (m_guardLights)
	{
		if (m_guardLightTime < guardLightMaxTime)
		{
			m_guardLightTime += dt;
			fpUpdateMotionAll(m_spEffectMotionGuardL.get(), dt);
			fpUpdateMotionAll(m_spEffectMotionGuardR.get(), dt);
		}
	}
	else
	{
		if (m_guardLightTime > 0.0f)
		{
			m_guardLightTime -= dt;
			fpUpdateMotionAll(m_spEffectMotionGuardL.get(), -dt);
			fpUpdateMotionAll(m_spEffectMotionGuardR.get(), -dt);
		}
	}

	// burner pfx
	if (m_guardLights && !m_guardLID && m_isLanded)
	{
		m_guardLID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spNodeGuardL, "ef_hover_guard", 1.0f);
		m_guardRID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spNodeGuardR, "ef_hover_guard", 1.0f);
	}
	else if (m_guardLID)
	{
		m_pGlitterPlayer->StopByID(m_guardLID, false);
		m_pGlitterPlayer->StopByID(m_guardRID, false);
		m_guardLID = 0;
		m_guardRID = 0;
	}
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
	hh::math::CVector forward = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ();
	if (!m_isLanded)
	{
		// not grounded, let gravity handle y-axis
		forward.y() = 0.0f;
		forward.normalize();
	}

	hh::math::CVector newPosition = m_spMatrixNodeTransform->m_Transform.m_Position;
	if (m_speed != 0.0f)
	{
		m_spProxy->m_Position = m_spMatrixNodeTransform->m_Transform.m_Position;
		m_spProxy->m_UpVector = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitY();
		m_spProxy->m_Velocity = forward * m_speed;
		Common::fCCharacterProxyIntegrate(m_spProxy.get(), dt);
		newPosition = m_spProxy->m_Position;
		m_speed = m_spProxy->m_Velocity.dot(forward);
	}

	// jumping
	if (m_jumpAccelTime > 0.0f)
	{
		m_isLanded = false;
		m_upSpeed += c_hoverJumpAccel * dt;

		// pfx
		if (!m_jumpID)
		{
			m_jumpID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spMatrixNodeTransform, "ef_hover_jump", 1.0f);
		}
	}
	m_jumpAccelTime = max(0.0f, m_jumpAccelTime - dt);

	// floor detection
	hh::math::CVector outPos = hh::math::CVector::Zero();
	hh::math::CVector outNormal = hh::math::CVector::UnitY();

	// hovering
	if (!m_isLanded)
	{
		if (m_jumpAccelTime == 0.0f)
		{
			m_upSpeed -= c_hoverGravity * dt;

			Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
			if (m_playerID && padState->IsDown(Sonic::EKeyState::eKeyState_A))
			{
				m_upSpeed = max(m_upSpeed, c_hoverJumpHoldSpeed);
			}
		}
		newPosition += hh::math::CVector::UnitY() * m_upSpeed * dt;
		
		// check landing
		if (m_upSpeed < 0.0f && Common::fRaycast(m_spMatrixNodeTransform->m_Transform.m_Position, newPosition, outPos, outNormal, m_landCollisionID))
		{
			SharedPtrTypeless sfx;
			Common::ObjectPlaySound(this, 200612012, sfx);

			// pfx
			hh::math::CVector temp1, temp2;
			bool const isWater = Common::fRaycast(m_spMatrixNodeTransform->m_Transform.m_Position, newPosition, temp1, temp2, *(int*)0x1E0AF9C);
			m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, isWater ? "ef_hover_water" : "ef_hover_land", 1.0f, 1);
			if (m_jumpID)
			{
				m_pGlitterPlayer->StopByID(m_jumpID, false);
				m_jumpID = 0;
			}

			newPosition = outPos;
			m_jumpAccelTime = 0.0f;
			m_upSpeed = 0.0f;
			m_isLanded = true;
		}

		// check ceiling
		hh::math::CVector const top = hh::math::CVector::UnitY() * 2.1f;
		if (m_upSpeed > 0.0f && Common::fRaycast(m_spMatrixNodeTransform->m_Transform.m_Position + top, newPosition + top, outPos, outNormal, m_landCollisionID))
		{
			newPosition = outPos - top;
			m_jumpAccelTime = 0.0f;
			m_upSpeed = 0.0f;
		}
	}
	else
	{
		// check leaving terrain
		hh::math::CVector const testStart = m_spMatrixNodeTransform->m_Transform.m_Position + hh::math::CVector(0.0f, 1.0f, 0.0f);
		hh::math::CVector const testEnd = m_spMatrixNodeTransform->m_Transform.m_Position + hh::math::CVector(0.0f, -0.1f, 0.0f);
		if (Common::fRaycast(testStart, testEnd, outPos, outNormal, m_landCollisionID))
		{
			newPosition.y() = outPos.y();

			// pitch
			float constexpr pitchRate = 5.0f;
			hh::math::CVector const upAxis = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitY();
			hh::math::CQuaternion const targetPitch = hh::math::CQuaternion::FromTwoVectors(upAxis.head<3>(), outNormal.head<3>());
			hh::math::CQuaternion const newRotation = hh::math::CQuaternion::Identity().slerp(dt * pitchRate, targetPitch) * m_spMatrixNodeTransform->m_Transform.m_Rotation;
			m_spMatrixNodeTransform->m_Transform.SetRotation(newRotation);
			m_spMatrixNodeTransform->NotifyChanged();
		}
		else
		{
			m_isLanded = false;

			// left ground, separate up/forward component
			hh::math::CVector forwardHorizontal = forward;
			forwardHorizontal.y() = 0.0f;
			forwardHorizontal.normalize();
			m_speed = forwardHorizontal.dot(forward * m_speed);
			m_upSpeed = hh::math::CVector::UnitY().dot(forward * m_speed);
		}
	}
	
	m_spMatrixNodeTransform->m_Transform.SetPosition(newPosition);
	m_spMatrixNodeTransform->NotifyChanged();

	// update loop sfx position
	if (m_loopSfx)
	{
		hh::math::CVector* pSoundHandle = (hh::math::CVector*)m_loopSfx.get();
		pSoundHandle[2] = newPosition;
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
}

void GadgetHover::ToggleBrakeLights(bool on)
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

void GadgetHover::TakeDamage(float amount)
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
