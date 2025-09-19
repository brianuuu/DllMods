#include "GadgetJeep.h"
#include "Character/NextGenShadow.h"

bool GadgetJeepBooster::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	std::string const name = "Gadget_Jeep_Booster";

	// model
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(name.c_str(), 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spNodeParent);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_castShadow);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, name.c_str());
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	std::string const revName = name + "_rev";
	entries.push_back(hh::anim::SMotionInfo("Boost", name.c_str(), 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo("Return", revName.c_str(), 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModel->BindPose(m_spAnimPose);

	// states
	SetContext(this);
	AddAnimationState("Boost");
	AddAnimationState("Return");
	ChangeState("Return");
	m_spAnimPose->Update(1.0f);

	SetCullingRange(0.0f);

	return true;
}

bool GadgetJeepBooster::ProcessMessage
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
			}

			return true;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);;
}

void GadgetJeepBooster::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	if (m_timer > 0.0f)
	{
		m_timer = max(0.0f, m_timer - in_rUpdateInfo.DeltaTime);
		if (m_timer == 0.0f)
		{
			ChangeState("Return");
		}
	}

	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);
}

bool GadgetJeepBooster::IsBoosting() const
{
	return GetCurrentState()->GetStateName() != "Return" || !Common::IsAnimationFinished(this);
}

float const c_jeepBoostTime = 2.0f;
bool GadgetJeepBooster::Boost()
{
	if (IsBoosting())
	{
		return false;
	}

	ChangeState("Boost");
	m_timer = c_jeepBoostTime;
	return true;
}

float GadgetJeepBooster::GetBoostTime() const
{
	return c_jeepBoostTime - m_timer;
}

GadgetJeep::GadgetJeep
(
	hh::mr::CTransform const& startTrans, 
	float speed
)
{
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(startTrans.m_Rotation, startTrans.m_Position);
	m_spMatrixNodeTransform->NotifyChanged();

	m_speed = speed;
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

	// guns & booster
	float constexpr c_jeepReloadTime = 2.0f;
	auto const attachNodeGunL = m_spModelBase->GetNode("GunUnder_L");
	m_spGunL = boost::make_shared<GadgetGunSimple>("Gadget_Jeep_GunL", attachNodeGunL, m_pMember->m_CastShadow, m_ActorID, c_jeepReloadTime);
	in_pGameDocument->AddGameObject(m_spGunL, "main", this);
	auto const attachNodeGunR = m_spModelBase->GetNode("GunUnder_R");
	m_spGunR = boost::make_shared<GadgetGunSimple>("Gadget_Jeep_GunR", attachNodeGunR, m_pMember->m_CastShadow, m_ActorID, c_jeepReloadTime);
	in_pGameDocument->AddGameObject(m_spGunR, "main", this);
	auto const attachNodeBooster = m_spModelBase->GetNode("Booster");
	m_spBooster = boost::make_shared<GadgetJeepBooster>(attachNodeBooster, m_pMember->m_CastShadow);
	in_pGameDocument->AddGameObject(m_spBooster, "main", this);

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
	fpGetMaterialAnimData(motWrapper, materialAnimData, "Gadget_Jeep_Reverse", 0);
	fpCreateMatAnim(m_spEffectMotionAll.get(), spModelBaseData, materialAnimData);

	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
	fpUpdateMotionAll(m_spEffectMotionAll.get(), 0.0f);

	SetCullingRange(0.0f);

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

	// Disable rigid body for a bit
	Common::ToggleRigidBodyCollision(m_spRigidBody.get(), false);
	m_collisionEnableTimer = 0.1f;

	// Wheel collisions
	hk2010_2_0::hkpCylinderShape* wheelLShape = new hk2010_2_0::hkpCylinderShape(hh::math::CVector::Zero(), hh::math::CVector(0.5f, 0.0f, 0.0f), 0.54f);
	hk2010_2_0::hkpCylinderShape* wheelRShape = new hk2010_2_0::hkpCylinderShape(hh::math::CVector::Zero(), hh::math::CVector(-0.5f, 0.0f, 0.0f), 0.54f);
	AddRigidBody(m_spRigidBodyWheelFL, wheelLShape, *(int*)0x1E0AFF4, m_spNodeGuardL);
	AddRigidBody(m_spRigidBodyWheelFR, wheelRShape, *(int*)0x1E0AFF4, m_spNodeGuardR);
	AddRigidBody(m_spRigidBodyWheelBL, wheelLShape, *(int*)0x1E0AFF4, m_spModelBase->GetNode("Wheel_B_L"));
	AddRigidBody(m_spRigidBodyWheelBR, wheelRShape, *(int*)0x1E0AFF4, m_spModelBase->GetNode("Wheel_B_R"));

	// damage to object
	m_spNodeCockpit = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeCockpit->m_Transform.SetPosition(hh::math::CVector(0.0f, 1.0f, -0.078f));
	m_spNodeCockpit->NotifyChanged();
	m_spNodeCockpit->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpBoxShape* cockpitEventTrigger = new hk2010_2_0::hkpBoxShape(1.4f, 1.4f, 4.0f);
	AddEventCollision("Enemy", cockpitEventTrigger, *(int*)0x1E0AF54 , true, m_spNodeCockpit); // ColID_TypeEnemy
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	AddEventCollision("Breakable", cockpitEventTrigger, Common::MakeCollisionID(0, (1llu << typeBreakable)), true, m_spNodeCockpit);

	// fake player collision
	hk2010_2_0::hkpCylinderShape* playerEventTrigger = new hk2010_2_0::hkpCylinderShape(hh::math::CVector(0.0f, 0.0f, -0.388f), hh::math::CVector(0.0f, 1.7f, -0.388f), 0.8f);
	AddEventCollision("FakePlayer", playerEventTrigger, *(int*)0x1E0AF90, true, m_spMatrixNodeTransform); // TypePlayer
	AddEventCollision("FakePlayerItem", playerEventTrigger, *(int*)0x1E0AF8C, true, m_spMatrixNodeTransform); // TypePlayerItem
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", false);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayerItem", false);

	// proxy collision
	uint32_t const typeTerrain = *(uint32_t*)0x1E5E754;
	uint32_t const typePlayerTerrain = *(uint32_t*)0x1E5E758;
	uint32_t const proxyID = Common::MakeCollisionID(0, (1llu << typeTerrain) | (1llu << typePlayerTerrain));
	Hedgehog::Base::THolder<Sonic::CWorld> holder(m_pMember->m_pWorld.get());
	hk2010_2_0::hkpBoxShape* proxyShape = new hk2010_2_0::hkpBoxShape(1.4f, 0.1f, 3.2f);
	m_spProxy = boost::make_shared<Sonic::CCharacterProxy>(this, holder, proxyShape, hh::math::CVector::UnitY() * 2.2f, hh::math::CQuaternion::Identity(), proxyID);

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

	m_rotation = m_spMatrixNodeTransform->m_Transform.m_Rotation;
	m_wheelFLPos = m_spNodeWheelFL->GetWorldMatrix().translation();
	m_wheelFRPos = m_spNodeWheelFR->GetWorldMatrix().translation();
	m_wheelBLPos = m_spNodeWheelBL->GetWorldMatrix().translation();
	m_wheelBRPos = m_spNodeWheelBR->GetWorldMatrix().translation();

	SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 80041038, 1);
	m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, "ef_jeep_warp", 1.0f, 1);
}

void GadgetJeep::KillCallback()
{
	BeginPlayerGetOff(false);
	NextGenShadow::m_vehicleSingleton.reset();
}

void GadgetJeep::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	AdvancePlayerGetOn(in_rUpdateInfo.DeltaTime);
	AdvanceDriving(in_rUpdateInfo.DeltaTime);
	AdvancePhysics(in_rUpdateInfo.DeltaTime);
}

bool GadgetJeep::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgRestartStage>() || message.Is<Sonic::Message::MsgStageClear>())
		{
			m_playerID = 0;
			CleanUp();
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
			return true;
		}

		if (message.Is<Sonic::Message::MsgExitedExternalControl>())
		{
			m_playerID = 0;
			CleanUp();
			NextGenShadow::m_vehicleSingleton.reset();
			Explode();
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
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
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

	float constexpr timeToGetOn = 1.0f;
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

	SendMessageImm(m_playerID, Sonic::Message::MsgFinishExternalControl(Sonic::Message::MsgFinishExternalControl::EChangeState::FALL));

	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	context->StateFlag(eStateFlag_EnableHomingAttack) = true;
	context->StateFlag(eStateFlag_EnableAirOnceAction) = true;

	hh::math::CVector velocity = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ() * m_speed;
	if (isAlive)
	{
		velocity.y() = context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_JumpPower);
		Common::SetPlayerVelocity(velocity);

		// Jump animation
		SharedPtrTypeless soundHandle;
		Common::SonicContextPlaySound(soundHandle, 2002027, 1);
		Common::SonicContextPlayVoice(soundHandle, 3002000, 0);
		FUNCTION_PTR(void*, __thiscall, ChangeAnimationCustomPlayback, 0xE74BF0, void* context, Hedgehog::Base::CSharedString const& name, hh::math::CVector const& change);
		ChangeAnimationCustomPlayback(context, "JumpBall", hh::math::CVector::Zero());
	}
	else
	{
		Common::SetPlayerVelocity(velocity);
	}

	// out of control
	Common::SetPlayerOutOfControl(0.1f);

	CleanUp();
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
	Common::ObjectPlaySound(this, 200612023, sfx);
	Common::ObjectPlaySound(this, 200612027, m_loopSfx);

	// load gun
	SendMessageImm(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	SendMessageImm(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	Common::ObjectPlaySound(this, 200612005, sfx);

	// Change animation
	SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("Jeep", true));

	// set HUD
	S06HUD_API::SetGadgetMaxCount(2);
	S06HUD_API::SetGadgetCount(2, 2);
	S06HUD_API::SetGadgetHP(m_hp);

	// player collision
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayer", true);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "FakePlayerItem", true);
}

float const c_jeepDoubleTapTime = 0.2f;
float const c_jeepWheelMaxAngle = 20.0f * DEG_TO_RAD;
float const c_jeepWheelTurnRate = c_jeepWheelMaxAngle / 0.5f;
float const c_jeepMaxSpeed = 30.0f;
float const c_jeepReverseSpeed = -12.0f;
float const c_jeepMinBrakeSpeed = 5.0f;
float const c_jeepAccel = 8.0f;
float const c_jeepBrake = 20.0f;
float const c_jeepDecel = 4.0f;
float const c_jeepGravity = 20.0f;
float const c_jeepBoostDashAccel = 20.0f;

void GadgetJeep::AdvanceDriving(float dt)
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

	Common::ClampFloat(m_speed, -c_jeepMaxSpeed, c_jeepMaxSpeed);

	m_outOfControl = max(0.0f, m_outOfControl - dt);
	if (m_state != State::Driving)
	{
		fnAccel(m_wheelAngle, 0.0f, c_jeepWheelTurnRate);
		fnAccel(m_speed, 0.0f, c_jeepAccel);
		return;
	}

	Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();

	// get off
	if (m_playerID && padState->IsTapped(Sonic::EKeyState::eKeyState_Y))
	{
		BeginPlayerGetOff(true);
		m_playerID = 0;
		Explode();
		return;
	}

	// calculate input, follows sub_F82000
	if (m_playerID)
	{
		m_input.x() = (abs(padState->LeftStickHorizontal) - 0.1f) / 0.8f;
		Common::ClampFloat(m_input.x(), 0.0f, 1.0f);
		if (padState->LeftStickHorizontal > 0.0f) m_input.x() *= -1.0f;
		m_input.y() = (abs(padState->LeftStickVertical) - 0.1f) / 0.8f;
		Common::ClampFloat(m_input.y(), 0.0f, 1.0f);
		if (padState->LeftStickVertical < 0.0f) m_input.y() *= -1.0f;
	}

	// rotation
	if (m_outOfControl == 0.0f && m_input.x() != 0.0f)
	{
		fnAccel(m_wheelAngle, m_input.x() * c_jeepWheelMaxAngle, c_jeepWheelTurnRate);
	}
	else
	{
		fnAccel(m_wheelAngle, 0.0f, c_jeepWheelTurnRate);
	}

	// acceleration
	bool shouldStopBrakeSfx = (m_speed <= c_jeepMinBrakeSpeed);
	if (m_isLanded)
	{
		if (m_spBooster->IsBoosting() && (m_spBooster->GetBoostTime() <= 1.0f || !padState->IsDown(Sonic::EKeyState::eKeyState_X)))
		{
			// boost dash
			fnAccel(m_speed, c_jeepMaxSpeed, c_jeepBoostDashAccel);
			ToggleBrakeLights(false);
			shouldStopBrakeSfx = true;
		}
		else if (m_playerID && m_outOfControl == 0.0f && padState->IsDown(Sonic::EKeyState::eKeyState_A))
		{
			// forward
			fnAccel(m_speed, c_jeepMaxSpeed, m_speed < 0.0f ? c_jeepBrake : c_jeepAccel);
			ToggleBrakeLights(false);
			shouldStopBrakeSfx = true;
		}
		else if (m_playerID && m_outOfControl == 0.0f && padState->IsDown(Sonic::EKeyState::eKeyState_X))
		{
			// brake, reverse
			fnAccel(m_speed, c_jeepReverseSpeed, m_speed > 0.0f ? c_jeepBrake : c_jeepAccel);
			ToggleBrakeLights(true);

			// brake sfx
			if (m_speed > c_jeepMinBrakeSpeed && !m_brakeSfx)
			{
				Common::ObjectPlaySound(this, 200612031, m_brakeSfx);
			}
		}
		else
		{
			// natural stop
			fnAccel(m_speed, 0.0f, c_jeepAccel);
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
	if (m_playerID && m_isLanded && m_outOfControl == 0.0f && padState->IsTapped(Sonic::EKeyState::eKeyState_A))
	{
		if (m_doubleTapTime > 0.0f)
		{
			if (m_spBooster->Boost())
			{
				SharedPtrTypeless sfx;
				Common::ObjectPlaySound(this, 200612030, sfx);

				m_pGlitterPlayer->PlayOneshot(m_spModelBase->GetNode("pBoost_L"), "ef_jeep_boost", 1.0f, 1);
				m_pGlitterPlayer->PlayOneshot(m_spModelBase->GetNode("pBoost_R"), "ef_jeep_boost", 1.0f, 1);
			}

			m_doubleTapTime = 0.0f;
		}
		else
		{
			m_doubleTapTime = c_jeepDoubleTapTime;
		}
	}

	// fire missiles
	if (m_playerID)
	{
		bool const rLoaded = m_spGunR->IsLoaded();
		bool const lLoaded = m_spGunL->IsLoaded();
		S06HUD_API::SetGadgetCount(rLoaded + lLoaded, 2);
		if (m_state == State::Driving && padState->IsTapped(Sonic::EKeyState::eKeyState_RightTrigger))
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

void GadgetJeep::AdvancePhysics(float dt)
{
	if (m_collisionEnableTimer > 0.0f)
	{
		m_collisionEnableTimer = max(0.0f, m_collisionEnableTimer - dt);
		if (m_collisionEnableTimer == 0.0f)
		{
			Common::ToggleRigidBodyCollision(m_spRigidBody.get(), true);
		}
	}

	hh::math::CVector const upAxis = hh::math::CVector::UnitY();
	m_speed = min(c_jeepMaxSpeed, m_speed);

	if (m_speed != 0.0f)
	{
		// vehicle yaw
		float const speedRatio = m_speed / c_jeepMaxSpeed;
		float constexpr steerScale = 10.0f;
		m_rotation = Eigen::AngleAxisf((m_isLanded ? m_wheelAngle : 0.0f) * speedRatio * steerScale * dt, upAxis) * m_rotation;

		// wheels
		float const sign = m_speed > 0.0f ? 1.0f : -1.0f;
		auto fnWheelSpin = [&dt, &sign](Sonic::CMatrixNodeTransform* transform, hh::math::CVector& pos)
		{
			hh::math::CVector const newPos = transform->GetWorldMatrix().translation();
			if ((newPos - pos).isApprox(hh::math::CVector::Zero()))
			{
				return;
			}

			float const dist = (newPos - pos).norm();
			float constexpr wheelRadius = 0.54f;
			float const theta = dist * sign / wheelRadius;
			hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(theta, hh::math::CVector::UnitX()) * transform->m_Transform.m_Rotation;
			transform->m_Transform.SetRotation(newRotation);
			transform->NotifyChanged();
			pos = newPos;
		};

		fnWheelSpin(m_spNodeWheelFL.get(), m_wheelFLPos);
		fnWheelSpin(m_spNodeWheelFR.get(), m_wheelFRPos);
		fnWheelSpin(m_spNodeWheelBL.get(), m_wheelBLPos);
		fnWheelSpin(m_spNodeWheelBR.get(), m_wheelBRPos);
	}

	{
		// guards
		hh::math::CQuaternion newRotation = Eigen::AngleAxisf(m_wheelAngle, upAxis) * hh::math::CQuaternion::Identity();
		m_spNodeGuardR->m_Transform.SetRotation(newRotation);
		m_spNodeGuardR->NotifyChanged();
		m_spNodeGuardL->m_Transform.SetRotation(newRotation);
		m_spNodeGuardL->NotifyChanged();
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

	// in-air
	if (!m_isLanded)
	{
		m_upSpeed -= c_jeepGravity * dt;
		newPosition += hh::math::CVector::UnitY() * m_upSpeed * dt;

		// check landing
		if (m_upSpeed < 0.0f && Common::fRaycast(testStart, newPosition, outPos, outNormal, *(int*)0x1E0AFAC))
		{
			SharedPtrTypeless sfx;
			Common::ObjectPlaySound(this, 200612026, sfx);

			newPosition = outPos;
			m_upSpeed = 0.0f;
			m_isLanded = true;
		}

		// check ceiling
		hh::math::CVector const top = hh::math::CVector::UnitY() * 1.8f;
		if (m_upSpeed > 0.0f && Common::fRaycast(testStart, newPosition + top, outPos, outNormal, *(int*)0x1E0AFAC))
		{
			newPosition = outPos - top;
			m_upSpeed = 0.0f;
		}
	}
	else
	{
		// check leaving terrain
		hh::math::CVector const testEnd = newPosition + hh::math::CVector(0.0f, -0.25f, 0.0f);
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

	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(m_rotation, newPosition);
	m_spMatrixNodeTransform->NotifyChanged();

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

		float value = 0.5f + (m_speed + abs(m_upSpeed)) * 0.5f / c_jeepMaxSpeed;
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

void GadgetJeep::ToggleBrakeLights(bool on)
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

void GadgetJeep::TakeDamage(float amount)
{
	std::lock_guard guard(m_mutex);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612025, sfx);

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
