#include "MephilesShadow.h"
#include "Mephiles.h"

float const MephilesShadow::c_DodgeSpeed = 2.0f;
float const MephilesShadow::c_ApproachSpeed = 5.0f;
float const MephilesShadow::c_EscapeSpeed = 10.0f;
float const MephilesShadow::c_MinEncirclementRadius = 8.0f;
float const MephilesShadow::c_MaxEncirclementRadius = 20.0f;
float const MephilesShadow::c_TargetLostDistance = 30.0f;
float const MephilesShadow::c_DeltaSpeed = 12.0f;
float const MephilesShadow::c_MinEncirclementHeight = 0.75f;
float const MephilesShadow::c_MaxEncirclementHeight = 3.5f;

char const* MephilesShadow::Loop = "Loop";
char const* MephilesShadow::BodyAttack = "BodyAttack";
char const* MephilesShadow::Catch = "Catch";
char const* MephilesShadow::CatchMiss = "CatchMiss";
char const* MephilesShadow::Dead = "Dead";

MephilesShadow::MephilesShadow
(
	uint32_t owner, 
	Type type,
	float radius,
	hh::math::CVector const& startPos
)
	: m_owner(owner)
	, m_type(type)
{
	m_startPos = startPos;
	hh::math::CVector adjustedPos = startPos;

	switch (type)
	{
	case Type::Encirclement:
	{
		radius = RAND_FLOAT(radius, c_TargetLostDistance);
		m_startPos.y() += RAND_FLOAT(c_MinEncirclementHeight, c_MaxEncirclementHeight);

		float const randAngle = RAND_FLOAT(0.0f, 2.0f * PI_F);
		hh::math::CVector const direction = Eigen::AngleAxisf(randAngle, Eigen::Vector3f::UnitY()) * hh::math::CVector::UnitZ();
		adjustedPos = m_startPos + direction * radius;
		break;
	}
	}

	m_spMatrixNodeTransform->m_Transform.SetPosition(adjustedPos);
	FaceDirection(GetPlayerDirection());
}

bool MephilesShadow::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// model
	char const* modelName = "en_zmef_Root";
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(modelName, 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, false);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, modelName);
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo(Loop, "en_wait_zmef_Root", 1.0f, hh::anim::eMotionRepeatType_Loop));
	entries.push_back(hh::anim::SMotionInfo(BodyAttack, "en_bodyattack_zmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo(Catch, "en_catch_zmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo(CatchMiss, "en_misscatch_zmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo(Dead, "en_dead_zmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModel->BindPose(m_spAnimPose);

	auto fnAddAnimationState = [this](hh::base::CSharedString name, hh::base::CSharedString target = "")
	{
		auto state = AddAnimationState(name);
		if (!target.empty())
		{
			state->m_TransitionState = target;
			state->m_Field8C = -1.0f;
			state->m_Field90 = 1;
		}
	};

	// states
	SetContext(this);
	fnAddAnimationState(Loop);
	fnAddAnimationState(BodyAttack);
	fnAddAnimationState(Catch);
	fnAddAnimationState(CatchMiss);
	fnAddAnimationState(Dead);
	SetAnimationBlend(BodyAttack, Loop, 0.5f);
	SetAnimationBlend(CatchMiss, Loop, 0.5f);
	ChangeState(Loop);

	SetCullingRange(0.0f);

	return true;
}

bool MephilesShadow::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	auto const& bodyCenter = m_spModel->GetNode("Spine");
	m_spNodeBody = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeBody->SetParent(bodyCenter.get());

	// rigid body
	hk2010_2_0::hkpSphereShape* rigidBodyShape = new hk2010_2_0::hkpSphereShape(0.4f);
	AddEventCollision("Body", rigidBodyShape, *(int*)0x1E0AF24, true, m_spNodeBody); // CollisionTypeEnemyAndLockOn

	// damage body
	hk2010_2_0::hkpSphereShape* damageShape = new hk2010_2_0::hkpSphereShape(0.4f);
	AddEventCollision("Damage", damageShape, *(int*)0x1E0AFD8, true, m_spNodeBody); // ColID_PlayerEvent

	// enemy evasion
	hk2010_2_0::hkpSphereShape* evasionShape = new hk2010_2_0::hkpSphereShape(1.0f);
	AddEventCollision("Enemy", evasionShape, *(int*)0x1E0AF54, true, m_spNodeBody); // ColID_TypeEnemy

	return true;
}

bool MephilesShadow::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgRestartStage>() || message.Is<Sonic::Message::MsgStageClear>())
		{
			Kill();
			return true;
		}

		if (message.Is<Sonic::Message::MsgDamage>())
		{
			auto& msg = static_cast<Sonic::Message::MsgDamage&>(message);
			SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamageSuccess>(msg.m_DamagePosition, true));

			float constexpr c_BlownSpeed = 12.0f;
			m_speed = c_BlownSpeed;
			if (msg.m_Velocity.isZero())
			{
				m_direction = (GetBodyPosition() - msg.m_DamagePosition).normalized();
			}
			else
			{
				m_direction = msg.m_Velocity.normalized();
			}

			m_stateNext = State::Blown;
			return true;
		}

		if (message.Is<Sonic::Message::MsgNotifyShockWave>())
		{
			SharedPtrTypeless soundHandle;
			Common::ObjectPlaySound(this, 200614000, soundHandle);

			m_stateTime = 0.0f;
			m_stateNext = State::Shock;
			return true;
		}

		if (message.Is<Sonic::Message::MsgHitEventCollision>())
		{
			auto& msg = static_cast<Sonic::Message::MsgHitEventCollision&>(message);
			if (CanDamagePlayer() && msg.m_Symbol == "Damage")
			{
				SendMessage(msg.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
					(
						*(uint32_t*)0x1E0BE34, // DamageID_NoAttack
						GetBodyPosition(),
						hh::math::CVector::Zero()
					)
				);
			}
			else if (msg.m_Symbol == "Terrain") // only in State::Blown
			{
				m_stateNext = State::Dead;
			}
			else if (msg.m_Symbol == "Enemy" && message.m_SenderActorID != m_ActorID)
			{
				m_escapeEnemies.push_back(message.m_SenderActorID);
			}
			return true;
		}

		if (message.Is<Sonic::Message::MsgLeaveEventCollision>())
		{
			auto& msg = static_cast<Sonic::Message::MsgLeaveEventCollision&>(message);
			if (msg.m_Symbol == "Enemy" && message.m_SenderActorID != m_ActorID)
			{
				m_escapeEnemies.erase(std::find(m_escapeEnemies.begin(), m_escapeEnemies.end(), message.m_SenderActorID));
			}
			return true;
		}

		if (message.Is<Sonic::Message::MsgGetEnemyType>())
		{
			auto& msg = static_cast<Sonic::Message::MsgGetEnemyType&>(message);
			*msg.m_pType = 2;
			return true;
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
			*msg.m_pPosition = GetBodyPosition();
			return true;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void MephilesShadow::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	switch (m_state)
	{
	case State::Idle: StateIdleAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::Shock: StateShockAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::Blown: StateBlownAdvance(in_rUpdateInfo.DeltaTime); break;
	}

	m_stateTime += in_rUpdateInfo.DeltaTime;
	if (m_stateNext != m_state)
	{
		HandleStateChange();
	}

	if (m_state != State::Shock)
	{
		m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
		Update(in_rUpdateInfo);
	}
}

void MephilesShadow::HandleStateChange()
{
	// end current state
	switch (m_state)
	{
	case State::Shock: StateShockEnd(); break;
	case State::Blown: StateBlownEnd(); break;
	}

	// start next state
	switch (m_stateNext)
	{
	case State::Shock: StateShockBegin(); break;
	case State::Blown: StateBlownBegin(); break;
	}

	m_state = m_stateNext;
	m_stateTime = 0.0f;
}

//---------------------------------------------------
// State::Idle
//---------------------------------------------------
void MephilesShadow::StateIdleAdvance(float dt)
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

	float distance = 0.0f;
	hh::math::CVector const dir = GetPlayerDirection(&distance);

	if (!m_targetLost && distance > c_TargetLostDistance)
	{
		// too far, get back into c_MaxEncirclementRadius
		m_targetLost = true;
	}

	if (m_targetLost && distance > c_MaxEncirclementRadius)
	{
		m_direction = dir;
		fnAccel(m_speed, c_ApproachSpeed, c_DeltaSpeed);
	}
	else if (distance < c_MinEncirclementRadius)
	{
		m_direction = -dir;
		fnAccel(m_speed, c_EscapeSpeed, c_DeltaSpeed);
	}
	else
	{
		fnAccel(m_speed, 0.0f, c_DeltaSpeed);
	}

	if (!m_escapeEnemies.empty())
	{
		// escape other shadows by moving perpendiular to player direction
		hh::math::CVector otherPos = hh::math::CVector::Zero();
		SendMessageImm(m_escapeEnemies.front(), Sonic::Message::MsgGetPosition(otherPos));
		hh::math::CVector const escapeDir = m_spMatrixNodeTransform->m_Transform.m_Position - otherPos;
		hh::math::CVector const dirLeft = Eigen::AngleAxisf(PI_F * 0.5f, Eigen::Vector3f::UnitY()) * dir;
		hh::math::CVector const newPosition = m_spMatrixNodeTransform->m_Transform.m_Position + (escapeDir.dot(dirLeft) > 0.0f ? dirLeft : -dirLeft) * c_DodgeSpeed * dt;
		m_spMatrixNodeTransform->m_Transform.SetPosition(newPosition);
		m_spMatrixNodeTransform->NotifyChanged();
	}

	UpdatePosition(dt);
	FaceDirection(dir);
}

void MephilesShadow::StateIdleEnd()
{
	m_targetLost = false;
}

//---------------------------------------------------
// State::Blown
//---------------------------------------------------
void MephilesShadow::StateBlownBegin()
{
	// notify owner it's about to die
	SendMessageImm(m_owner, Sonic::Message::MsgNotifyObjectEvent(0));

	SharedPtrTypeless soundHandle;
	Common::ObjectPlaySound(this, 200614000, soundHandle);

	ChangeState(Dead);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Body", false);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Damage", false);

	// terrain/enemy detection
	hk2010_2_0::hkpSphereShape* damageShape = new hk2010_2_0::hkpSphereShape(0.3f);
	AddEventCollision("Terrain", damageShape, *(int*)0x1E0AFA4, true, m_spNodeBody); // TypeTerrain + TypeWater
}

void MephilesShadow::StateBlownAdvance(float dt)
{
	UpdatePosition(dt);

	float constexpr c_BlownTime = 0.25f;
	if (m_stateTime >= c_BlownTime || m_speed == 0.0f)
	{
		m_stateNext = State::Dead;
	}
}

void MephilesShadow::StateBlownEnd()
{
	// TODO: sfx, pfx
	Common::SpawnBoostParticle((uint32_t**)this, m_spMatrixNodeTransform->m_Transform.m_Position, 0x10001);
	Kill();
}

//---------------------------------------------------
// State::Shock
//---------------------------------------------------
void MephilesShadow::StateShockBegin()
{
	Common::ObjectPlaySound(this, 200614003, m_shockSfx);
	m_shockID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spModel->GetNode("Spine"), "ef_ch_sns_yh1_damage_shock2", 1.0f);
}

void MephilesShadow::StateShockAdvance(float dt)
{
	float constexpr c_ParalysisTime = 5.0f;
	if (m_stateTime >= c_ParalysisTime)
	{
		m_stateNext = State::Idle;
	}
}

void MephilesShadow::StateShockEnd()
{
	m_shockSfx.reset();
	if (m_shockID)
	{
		m_pGlitterPlayer->StopByID(m_shockID, false);
		m_shockID = 0;
	}
}

//---------------------------------------------------
// Utils
//---------------------------------------------------
hh::math::CVector MephilesShadow::GetBodyPosition() const
{
	return m_spModel->GetNode("Spine")->GetWorldMatrix().translation();
}

bool MephilesShadow::CanDamagePlayer() const
{
	return m_type == Type::Encirclement;
}

void MephilesShadow::UpdatePosition(float dt)
{
	hh::math::CVector newPosition = m_spMatrixNodeTransform->m_Transform.m_Position + m_direction * m_speed * dt;
	if (newPosition.y() < m_startPos.y())
	{
		newPosition.y() = min(m_startPos.y(), newPosition.y() + c_DodgeSpeed * dt);
	}
	else if(newPosition.y() > m_startPos.y())
	{
		newPosition.y() = max(m_startPos.y(), newPosition.y() - c_DodgeSpeed * dt);
	}

	m_spMatrixNodeTransform->m_Transform.SetPosition(newPosition);
	m_spMatrixNodeTransform->NotifyChanged();
}

void MephilesShadow::FaceDirection(hh::math::CVector const& dir)
{
	hh::math::CQuaternion const rotation = hh::math::CQuaternion::FromTwoVectors(hh::math::CVector::UnitZ(), dir.head<3>());
	m_spMatrixNodeTransform->m_Transform.SetRotation(rotation);
	m_spMatrixNodeTransform->NotifyChanged();
}

hh::math::CVector MephilesShadow::GetPlayerDirection(float* distance) const
{
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector const playerPos = context->m_spMatrixNode->m_Transform.m_Position;
	hh::math::CVector dir = playerPos - m_spMatrixNodeTransform->m_Transform.m_Position;
	if (distance) *distance = dir.norm();
	dir.y() = 0.0f;
	dir.normalize();
	return dir;
}
