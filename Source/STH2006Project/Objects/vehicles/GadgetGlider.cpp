#include "GadgetGlider.h"
#include "GadgetMissile.h"

bool GadgetGliderGun::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// model
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(m_modelName.c_str(), 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spNodeParent);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_castShadow);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, m_modelName.c_str());
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	std::string const revName = m_modelName + "_rev";
	entries.push_back(hh::anim::SMotionInfo("Load", m_modelName.c_str(), 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo("Fire", revName.c_str(), 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModel->BindPose(m_spAnimPose);

	// states
	SetContext(this);
	AddAnimationState("Load");
	AddAnimationState("Fire");
	ChangeState("Load");

	// set initial transform
	UpdateTransform();

	return true;
}

bool GadgetGliderGun::ProcessMessage
(
	Hedgehog::Universe::Message& message,
	bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
		{
			auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
			switch (msg.m_Event)
			{
			case 6: 
			{
				m_started = true;
				break;
			}
			case 0:
			{
				FireMissile();
				break;
			}
			}
			return true;
		}
	}

	return Sonic::CGameObject3D::ProcessMessage(message, flag);
}

float const c_gliderReloadTime = 1.0f;

void GadgetGliderGun::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	if (!m_started) return;

	if (m_loadTimer > 0.0f)
	{
		if (GetCurrentState()->GetStateName() != "Fire")
		{
			ChangeState("Fire");
		}

		m_loadTimer = max(0.0f, m_loadTimer - in_rUpdateInfo.DeltaTime);
		if (m_loadTimer == 0.0f)
		{
			ChangeState("Load");

			SharedPtrTypeless sfx;
			Common::ObjectPlaySound(this, 200612005, sfx);
		}
	}

	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);
	UpdateTransform();
}

bool GadgetGliderGun::IsLoaded()
{
	return m_loadTimer <= 0.0f && GetCurrentState()->GetStateName() == "Load" && Common::IsAnimationFinished(this);
}

void GadgetGliderGun::FireMissile()
{
	if (m_loadTimer > 0.0f) return;
	m_loadTimer = c_gliderReloadTime;

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612006, sfx);

	auto node = m_spModel->GetNode("MissilePoint");
	hh::mr::CTransform startTrans;
	startTrans.m_Rotation = hh::math::CQuaternion(node->GetWorldMatrix().rotation());
	startTrans.m_Position = node->GetWorldMatrix().translation();
	m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<GadgetMissile>(m_owner, startTrans));
}

void GadgetGliderGun::UpdateTransform()
{
	// follow attach point so sound can work
	hh::math::CMatrix const matrix = m_spNodeParent->GetWorldMatrix();
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(hh::math::CQuaternion(matrix.rotation()), matrix.translation());
	m_spMatrixNodeTransform->NotifyChanged();
}

uint32_t canGetOnGliderActorID = 0u;
HOOK(bool, __fastcall, GadgetGlider_GroundedStateChange, 0xE013D0, Sonic::Player::CPlayerSpeedContext* context, void* Edx, int a2)
{
	if (context->m_Grounded && !context->StateFlag(eStateFlag_OutOfControl))
	{
		if (canGetOnGliderActorID && Common::fIsButtonTapped(Sonic::EKeyState::eKeyState_Y))
		{
			context->m_pPlayer->SendMessageImm(canGetOnGliderActorID, Sonic::Message::MsgNotifyObjectEvent(6));
			return true;
		}
	}

	return originalGadgetGlider_GroundedStateChange(context, Edx, a2);
}

BB_SET_OBJECT_MAKE_HOOK(GadgetGlider)
void GadgetGlider::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(GadgetGlider);
	applyPatches();
}

void GadgetGlider::applyPatches()
{
	INSTALL_HOOK(GadgetGlider_GroundedStateChange);
}

GadgetGlider::~GadgetGlider()
{
	if (m_loopSfx)
	{
		m_loopSfx.reset();
	}
}

void GadgetGlider::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
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
	m_spModelBase = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModelBase->BindMatrixNode(m_spMatrixNodeTransform);
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
	auto const attachNodeGunL = m_spModelBase->GetNode("GunUnder_L");
	m_spGunL = boost::make_shared<GadgetGliderGun>("Gadget_Glider_GunL", attachNodeGunL, m_pMember->m_CastShadow, m_ActorID);
	in_pGameDocument->AddGameObject(m_spGunL, "main", this);
	auto const attachNodeGunR = m_spModelBase->GetNode("GunUnder_R");
	m_spGunR = boost::make_shared<GadgetGliderGun>("Gadget_Glider_GunR", attachNodeGunR, m_pMember->m_CastShadow, m_ActorID);
	in_pGameDocument->AddGameObject(m_spGunR, "main", this);

	// external control
	auto const attachNode = m_spModelBase->GetNode("Charapoint");
	m_spSonicControlNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spSonicControlNode->SetParent(attachNode.get());

	return true;
}

bool GadgetGlider::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body
	char const* rigidBodyName = "Gadget_Glider";
	AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x1E0AFF4, m_spMatrixNodeTransform, in_spDatabase);

	// damage to object
	uint32_t const typeEnemy = *(uint32_t*)0x1E5E7E8;
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	uint32_t const typeTerrain = *(uint32_t*)0x1E5E754;
	uint32_t const typeInsulate = *(uint32_t*)0x1E5E780;
	uint64_t const bitfield = (1llu << typeEnemy) | (1llu << typeBreakable) | (1llu << typeTerrain);
	uint32_t const damageID = Common::MakeCollisionID(0, bitfield);
	hk2010_2_0::hkpBoxShape* bodyEventTrigger = new hk2010_2_0::hkpBoxShape(4.9f, 0.7f, 2.0f);
	AddEventCollision("Attack", bodyEventTrigger, damageID, true, m_spMatrixNodeTransform);
	AddRigidBody(m_spRigidBodyMove, bodyEventTrigger, Common::MakeCollisionID((1llu << typeInsulate), 0), m_spMatrixNodeTransform);

	m_spNodeCockpit = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeCockpit->m_Transform.SetPosition(hh::math::CVector(0.0f, -0.6f, 0.0f));
	m_spNodeCockpit->NotifyChanged();
	m_spNodeCockpit->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpBoxShape* cockpitEventTrigger = new hk2010_2_0::hkpBoxShape(0.8f, 0.5f, 1.5f);
	AddEventCollision("Attack", cockpitEventTrigger, damageID, true, m_spNodeCockpit);
	AddRigidBody(m_spRigidBodyCockpit, cockpitEventTrigger, Common::MakeCollisionID((1llu << typeInsulate), 0), m_spNodeCockpit);

	// player event collision
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEventCollision->m_Transform.SetPosition(hh::math::CVector(0.0f, -0.8f, 0.0f));
	m_spNodeEventCollision->NotifyChanged();
	m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpSphereShape* shapeEventTrigger = new hk2010_2_0::hkpSphereShape(2.0f);
	AddEventCollision("Player", shapeEventTrigger, *(int*)0x1E0AFD8, true, m_spNodeEventCollision); // ColID_PlayerEvent

	return true;
}

void GadgetGlider::KillCallback()
{
	if (m_playerID)
	{
		SendMessageImm(m_playerID, Sonic::Message::MsgFinishExternalControl(Sonic::Message::MsgFinishExternalControl::EChangeState::FALL));
		S06HUD_API::SetGadgetMaxCount(0);
	}
}

float const c_gliderAccel = 10.0f;
float const c_gliderMaxSpeed = 10.0f;
float const c_gliderBoostSpeed = 21.0f;
float const c_gliderMaxSteer = 5.0f;
float const c_gliderSteerRate = 10.0f;
float const c_gliderSteerToAngle = 4.5f * DEG_TO_RAD;
float const c_gliderGetOnTime = 1.0f;

void GadgetGlider::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	AdvancePlayerGetOn(in_rUpdateInfo.DeltaTime);

	if (!m_started) return;
	
	// counterweight animation
	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime * m_speed * 0.4f);

	// helper function
	auto fnAccel = [&in_rUpdateInfo](float& value, float target, float accel)
	{
		if (value > target)
		{
			value = max(target, value - accel * in_rUpdateInfo.DeltaTime);
		}
		else if (value < target)
		{
			value = min(target, value + accel * in_rUpdateInfo.DeltaTime);
		}
	};

	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();

	// fire missiles
	if (m_playerID)
	{
		bool const rLoaded = m_spGunR->IsLoaded();
		bool const lLoaded = m_spGunL->IsLoaded();
		S06HUD_API::SetGadgetCount(rLoaded + lLoaded, 2);
		if (padState->IsTapped(Sonic::EKeyState::eKeyState_RightTrigger))
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

	// boost max speed
	float currentMaxSpeed = c_gliderMaxSpeed;
	if (m_playerID && padState->IsDown(Sonic::EKeyState::eKeyState_A))
	{
		currentMaxSpeed = c_gliderBoostSpeed;
	}

	// steering x-axis
	if (m_playerID && ((context->m_WorldInput.x() > 0.0f && m_offset.x() > -m_Data.m_Radius) || (context->m_WorldInput.x() < 0.0f && m_offset.x() < m_Data.m_Radius)))
	{
		m_steer.x() += -context->m_WorldInput.x() * c_gliderSteerRate * in_rUpdateInfo.DeltaTime;
		Common::ClampFloat(m_steer.x(), -c_gliderMaxSteer, c_gliderMaxSteer);
	}
	else
	{
		fnAccel(m_steer.x(), 0.0f, c_gliderSteerRate);
	}

	// steering y-axis
	if (m_playerID && ((context->m_WorldInput.z() > 0.0f && m_offset.y() > -m_Data.m_Radius) || (context->m_WorldInput.z() < 0.0f && m_offset.y() < m_Data.m_Radius)))
	{
		m_steer.y() += -context->m_WorldInput.z() * c_gliderSteerRate * in_rUpdateInfo.DeltaTime;
		Common::ClampFloat(m_steer.y(), -c_gliderMaxSteer, c_gliderMaxSteer);
	}
	else
	{
		fnAccel(m_steer.y(), 0.0f, c_gliderSteerRate);
	}

	// roll, yaw, pitch
	hh::math::CVector const upAxis = m_rotation * hh::math::CVector::UnitY();
	hh::math::CVector const forward = m_rotation * hh::math::CVector::UnitZ();
	hh::math::CVector const rightAxis = m_rotation * hh::math::CVector::UnitX();
	hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(m_steer.y() * c_gliderSteerToAngle, -rightAxis) * Eigen::AngleAxisf(m_steer.x() * c_gliderSteerToAngle, upAxis) * Eigen::AngleAxisf(m_steer.x() * c_gliderSteerToAngle, -forward) * m_rotation;

	// move along path
	m_offset += m_steer * in_rUpdateInfo.DeltaTime;
	fnAccel(m_speed, currentMaxSpeed, c_gliderAccel);
	m_splinePos += forward * m_speed * in_rUpdateInfo.DeltaTime;
	hh::math::CVector const newPosition = m_splinePos + upAxis * m_offset.y() + rightAxis * m_offset.x();

	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(newRotation, newPosition);
	m_spMatrixNodeTransform->NotifyChanged();

	hh::math::CQuaternion const boosterLRotation = Eigen::AngleAxisf(m_steer.x() * c_gliderSteerToAngle * 1.2f, hh::math::CVector::UnitX()) * hh::math::CQuaternion::Identity();
	m_spNodeBoosterL->m_Transform.SetRotation(boosterLRotation);
	m_spNodeBoosterL->NotifyChanged();
	hh::math::CQuaternion const boosterRRotation = Eigen::AngleAxisf(m_steer.x() * c_gliderSteerToAngle * 1.2f, -hh::math::CVector::UnitX()) * hh::math::CQuaternion::Identity();
	m_spNodeBoosterR->m_Transform.SetRotation(boosterRRotation);
	m_spNodeBoosterR->NotifyChanged();

	// player animation
	if (m_playerID)
	{
		Direction direction = GetAnimationDirection(hh::math::CVector2(-context->m_WorldInput.x(), -context->m_WorldInput.z()));
		if (m_direction != direction)
		{
			m_direction = direction;
			SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl(GetAnimationName().c_str()));
		}
	}

	// update loop sfx position
	if (m_loopSfx)
	{
		hh::math::CVector* pSoundHandle = (hh::math::CVector*)m_loopSfx.get();
		pSoundHandle[2] = newPosition;
	}
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

	if (message.Is<Sonic::Message::MsgDamage>())
	{
		if (!SendMessageImm(message.m_SenderActorID, Sonic::Message::MsgGetPlayerType()))
		{
			TakeDamage(6.0f);
		}
		return true;
	}

	if (message.Is<Sonic::Message::MsgGetHomingAttackPriority>())
	{
		if (!IsValidPlayer()) return false;

		auto& msg = static_cast<Sonic::Message::MsgGetHomingAttackPriority&>(message);
		*msg.m_pPriority = 10;
		return true;
	}

	if (message.Is<Sonic::Message::MsgGetHomingAttackPosition>())
	{
		if (!IsValidPlayer()) return false;

		auto& msg = static_cast<Sonic::Message::MsgGetHomingAttackPosition&>(message);
		*msg.m_pPosition = m_spSonicControlNode->GetWorldMatrix().translation();
		return true;
	}

	if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
	{
		auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
		if (msg.m_Event == 6 && !m_started)
		{
			canGetOnGliderActorID = 0u;
			m_playerID = message.m_SenderActorID;
			m_playerGetOnData.m_gettingOn = true;

			hh::math::CVector playerPosition;
			SendMessageImm(m_playerID, Sonic::Message::MsgGetPosition(playerPosition));

			m_playerGetOnData.m_start = playerPosition - m_spSonicControlNode->GetWorldMatrix().translation();
			m_playerGetOnData.m_start.x() *= -1.0f;
			m_playerGetOnData.m_start.z() *= -1.0f;
			m_spSonicControlNode->m_Transform.SetPosition(m_playerGetOnData.m_start);
			m_spSonicControlNode->NotifyChanged();

			// Jump sfx
			SharedPtrTypeless soundHandle;
			Common::SonicContextPlaySound(soundHandle, 2002027, 1);
			Common::SonicContextPlayVoice(soundHandle, 3002000, 0);

			// start external control
			auto msgStartExternalControl = Sonic::Message::MsgStartExternalControl(m_spSonicControlNode, false, false);
			msgStartExternalControl.NoDamage = true;
			SendMessageImm(m_playerID, msgStartExternalControl);
			SendMessageImm(m_playerID, Sonic::Message::MsgChangeMotionInExternalControl("JumpBall", true));

			//BeginFlight();
		}

		return true;
	}

	if (message.Is<Sonic::Message::MsgHitEventCollision>())
	{
		auto& msg = static_cast<Sonic::Message::MsgHitEventCollision&>(message);
		if (msg.m_Symbol == "Player")
		{
			if (!m_started && IsValidPlayer())
			{
				canGetOnGliderActorID = m_ActorID;
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
						m_rotation * hh::math::CVector::UnitZ() * m_speed * 2.0f
					)
				);
			}
		}
		return true;
	}

	if (message.Is<Sonic::Message::MsgLeaveEventCollision>())
	{
		auto& msg = static_cast<Sonic::Message::MsgLeaveEventCollision&>(message);
		if (msg.m_Symbol == "Player" && !m_started)
		{
			canGetOnGliderActorID = 0;
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

bool GadgetGlider::IsValidPlayer() const
{
	return *pModernSonicContext && S06DE_API::GetModelType() == S06DE_API::ModelType::Shadow;
}

void GadgetGlider::BeginFlight()
{
	if (m_started) return;

	m_started = true;
	SendMessage(m_spGunR->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	SendMessage(m_spGunL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));

	m_splinePos = m_spMatrixNodeTransform->m_Transform.m_Position;
	m_rotation = m_spMatrixNodeTransform->m_Transform.m_Rotation;

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
	S06HUD_API::SetGadgetHP(m_hp);
	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612005, sfx);
}

void GadgetGlider::TakeDamage(float amount)
{
	std::lock_guard guard(m_mutex);

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612004, sfx);

	m_hp -= amount;

	if (m_playerID && m_started)
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
	// TODO: sfx pfx

	Kill();
}

void GadgetGlider::AdvancePlayerGetOn(float dt)
{
	if (!m_playerGetOnData.m_gettingOn) return;

	float const timeToGetOn = m_playerGetOnData.m_start.y() < 0.0f ? 0.2f : 2.0f;
	m_playerGetOnData.m_time += dt;

	if (m_playerGetOnData.m_time >= timeToGetOn)
	{
		m_playerGetOnData.m_gettingOn = false;
		m_spSonicControlNode->m_Transform.SetPosition(hh::math::CVector::Zero());
		m_spSonicControlNode->NotifyChanged();

		BeginFlight();
		return;
	}

	float const prop = m_playerGetOnData.m_time / timeToGetOn;
	hh::math::CVector targetPosition = m_playerGetOnData.m_start * (1.0f - prop);

	if (m_playerGetOnData.m_start.y() >= 0.0f)
	{
		float constexpr initSpeed = 10.0f;
		float const accel = initSpeed * -2.0f / timeToGetOn;
		float const yOffset = initSpeed * m_playerGetOnData.m_time + 0.5f * accel * m_playerGetOnData.m_time * m_playerGetOnData.m_time;
		targetPosition.y() += yOffset;
	}

	m_spSonicControlNode->m_Transform.SetPosition(targetPosition);
	m_spSonicControlNode->NotifyChanged();
}

GadgetGlider::Direction GadgetGlider::GetAnimationDirection(hh::math::CVector2 input) const
{
	float constexpr threshold = 0.2f;
	float const absX = abs(input.x());
	float const absY = abs(input.y());

	if (absX > threshold)
	{
		//if (absY > threshold && absY > absX)
		//{
		//	return input.y() > 0.0f ? Direction::Up : Direction::Down;
		//}
		//else
		{
			return input.x() > 0.0f ? Direction::Left : Direction::Right;
		}
	}
	
	if (absY > threshold)
	{
		if (absX > threshold && absX >= absY)
		{
			return input.x() > 0.0f ? Direction::Left : Direction::Right;
		}
		//else
		//{
		//	return input.y() > 0.0f ? Direction::Up : Direction::Down;
		//}
	}

	return Direction::None;
}

std::string GadgetGlider::GetAnimationName() const
{
	switch (m_direction)
	{
	//case Direction::Up: return "GliderU";
	//case Direction::Down: return "GliderD";
	case Direction::Left: return "GliderL";
	case Direction::Right: return "GliderR";
	default: return "Glider";
	}
}
