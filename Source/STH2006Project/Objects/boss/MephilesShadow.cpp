#include "MephilesShadow.h"
#include "Mephiles.h"

float const MephilesShadow::c_DodgeSpeed = 2.0f;
float const MephilesShadow::c_ApproachSpeed = 5.0f;
float const MephilesShadow::c_EscapeSpeed = 10.0f;
float const MephilesShadow::c_MinEncirclementRadius = 10.0f;
float const MephilesShadow::c_MaxEncirclementRadius = 20.0f;
float const MephilesShadow::c_TargetLostDistance = 30.0f;
float const MephilesShadow::c_DeltaSpeed = 12.0f;
float const MephilesShadow::c_MinEncirclementHeight = 0.75f;
float const MephilesShadow::c_MaxEncirclementHeight = 3.5f;
float const MephilesShadow::c_MinSpringAppearHeight = 1.0f;
float const MephilesShadow::c_MaxSpringAppearHeight = 2.5f;
float const MephilesShadow::c_CircularFlightSpeed = 0.5f * DEG_TO_RAD;
float const MephilesShadow::c_SpringSpeed = 5.0f;
float const MephilesShadow::c_SpringG = -9.8f;
float const MephilesShadow::c_SpringErrorRadius = 2.0f;
float const MephilesShadow::c_SpringFailedTime = 0.25f;
float const MephilesShadow::c_ChargeStartTime = 0.5f;
float const MephilesShadow::c_ChargeErrorRadius = 5.0f;
float const MephilesShadow::c_ChargeSpeed = 10.0f;
float const MephilesShadow::c_BlownSpeed = 12.0f;;

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
	float startAngle, 
	float groundHeight,
	hh::math::CVector const& startPos
)
	: m_owner(owner)
	, m_type(type)
	, m_groundHeight(groundHeight)
{
	hh::math::CVector adjustedPos = startPos;

	m_startPos = startPos;
	m_startPos.y() = m_groundHeight + RAND_FLOAT(c_MinEncirclementHeight, c_MaxEncirclementHeight);

	hh::math::CVector const direction = Eigen::AngleAxisf(startAngle, Eigen::Vector3f::UnitY()) * hh::math::CVector::UnitZ();

	switch (type)
	{
	case Type::Encirclement:
	{
		adjustedPos = m_startPos + direction * (RAND_FLOAT(radius, c_TargetLostDistance));
		break;
	}
	case Type::Spring:
	{
		adjustedPos += direction * radius;
		adjustedPos.y() += RAND_FLOAT(c_MinSpringAppearHeight, c_MaxSpringAppearHeight);
		break;
	}
	case Type::Charge:
	{
		adjustedPos += direction * radius;
		adjustedPos.y() += RAND_FLOAT(-0.5f, 0.5f);
		break;
	}
	}
	m_spNodeModel = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModel->SetParent(m_spMatrixNodeTransform.get());

	m_spMatrixNodeTransform->m_Transform.SetPosition(adjustedPos);
	FaceDirection(GetPlayerDirection(true));
}

void MephilesShadow::SetAddUpdateUnit
(
	Sonic::CGameDocument* in_pGameDocument
)
{
	in_pGameDocument->AddUpdateUnit("0", this);
	in_pGameDocument->AddUpdateUnit("1", this);
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
	m_spModel->BindMatrixNode(m_spNodeModel);
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
	SetAnimationBlend(Loop, BodyAttack, 0.5f);
	SetAnimationBlend(BodyAttack, Loop, 0.5f);
	SetAnimationBlend(Loop, Catch, 0.5f);
	SetAnimationBlend(Catch, Loop, 0.5f);
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
	AddEventCollision("Player", damageShape, *(int*)0x1E0AFD8, true, m_spNodeBody); // ColID_PlayerEvent

	// proxy collision
	uint32_t const typeKeepOffEnemy = *(uint32_t*)0x1E5E804;
	uint32_t const typeTerrain = *(uint32_t*)0x1E5E754;
	uint32_t const proxyCollisionID = Common::MakeCollisionID((1llu << typeKeepOffEnemy), (1llu << typeKeepOffEnemy) | (1llu << typeTerrain));
	Hedgehog::Base::THolder<Sonic::CWorld> holder(m_pMember->m_pWorld.get());
	hk2010_2_0::hkpSphereShape* proxyShape = new hk2010_2_0::hkpSphereShape(0.6f);
	m_spProxy = boost::make_shared<Sonic::CCharacterProxy>(this, holder, proxyShape, hh::math::CVector::UnitY() * 0.5f, hh::math::CQuaternion::Identity(), proxyCollisionID);

	return true;
}

void MephilesShadow::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	m_pGlitterPlayer->PlayOneshot(m_spModel->GetNode("Spine"), "ef_mephiles_appear", 1.0f, 1);
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
			SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamageSuccess>(msg.m_DamagePosition, true, true));

			// calculate blown direction
			if (msg.m_Velocity.isZero())
			{
				m_direction = (GetBodyPosition() - msg.m_DamagePosition).normalized();
			}
			else
			{
				m_direction = msg.m_Velocity.normalized();
			}

			PlayHitEffect();
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
			if (msg.m_Symbol == "Player")
			{
				if (m_state == State::SpringAttack)
				{
					if (S06DE_API::GetChaosBoostLevel() > 0)
					{
						// blown if chaos boost is active
						PlayHitEffect();
						m_direction = -GetPlayerDirection(true);
						m_stateNext = State::Blown;
					}
					else
					{
						// ask owner if this can attach to player
						SendMessage(m_owner, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(1));
					}
				}
				else if (CanDamagePlayer())
				{
					SendMessage(msg.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
						(
							*(uint32_t*)0x1E0BE34, // DamageID_NoAttack
							GetBodyPosition(),
							hh::math::CVector::Zero()
						)
					);
				}
			}
			else if (msg.m_Symbol == "Terrain") // only in State::Blown
			{
				m_stateNext = State::Dead;
			}
			return true;
		}

		if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
		{
			auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
			switch (msg.m_Event)
			{
			case 0:
			{
				// owner asked it to be killed (with effect)
				GetKilled(true, false, false);
				break;
			}
			case 1:
			{
				// allowed to attach to player
				m_stateNext = State::SpringAttach;
				break;
			}
			case 2:
			{
				// play suicide effect
				m_suicideID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spModel->GetNode("Spine"), "ef_mephiles_suicide", 1.0f);
				break;
			}
			case 3:
			{
				m_avoidOwner = true;
				break;
			}
			case 4:
			{
				m_avoidOwner = false;
				break;
			}
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
	// Non-time dependent update
	if (in_rUpdateInfo.Category == "1")
	{
		switch (m_state)
		{
		case State::SpringAttach: StateSpringAttachAdvance(in_rUpdateInfo.DeltaTime); break;
		}
		return;
	}

	// Time dependent update
	if (m_stateNext != m_state)
	{
		HandleStateChange();
	}

	switch (m_state)
	{
	case State::Idle: StateIdleAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::Shock: StateShockAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::Blown: StateBlownAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::CircleWait: StateCircleWaitAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::SpringAttack: StateSpringAttackAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::SpringMiss: StateSpringMissAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::ChargeAttack: StateChargeAttackAdvance(in_rUpdateInfo.DeltaTime); break;
	}

	m_stateTime += in_rUpdateInfo.DeltaTime;
	m_enableDamageTimer = max(0.0f, m_enableDamageTimer - in_rUpdateInfo.DeltaTime);

	if (m_state != State::Shock)
	{
		m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
		Update(in_rUpdateInfo);
	}
}

void MephilesShadow::SetInitialStateSpring(float attackStartTime, float attackMaxDelay)
{
	m_stateNext = State::CircleWait;
	m_attackStartTime = attackStartTime + RAND_FLOAT(0.0f, attackMaxDelay);
}

void MephilesShadow::HandleStateChange()
{
	// end current state
	switch (m_state)
	{
	case State::Shock: StateShockEnd(); break;
	case State::Blown: StateBlownEnd(); break;
	case State::CircleWait: StateCircleWaitEnd(); break;
	case State::SpringAttack: StateSpringAttackEnd(); break;
	case State::ChargeAttack: StateChargeAttackEnd(); break;
	}

	// start next state
	switch (m_stateNext)
	{
	case State::Idle: StateIdleBegin(); break;
	case State::Shock: StateShockBegin(); break;
	case State::Blown: StateBlownBegin(); break;
	case State::CircleWait: StateCircleWaitBegin(); break;
	case State::SpringAttack: StateSpringAttackBegin(); break;
	case State::SpringMiss: StateSpringMissBegin(); break;
	case State::SpringAttach: StateSpringAttachBegin(); break;
	case State::ChargeAttack: StateChargeAttackBegin(); break;
	}

	m_state = m_stateNext;
	m_stateTime = 0.0f;
}

//---------------------------------------------------
// State::Idle
//---------------------------------------------------
void MephilesShadow::StateIdleBegin()
{
	m_type = Type::Encirclement;
	ChangeState(Loop);
}

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
	m_direction = GetPlayerDirection(true, &distance);

	if (m_avoidOwner)
	{
		hh::math::CVector ownerDiff = hh::math::CVector::Zero();
		SendMessageImm(m_owner, Sonic::Message::MsgGetPosition(&ownerDiff));
		ownerDiff -= m_spMatrixNodeTransform->m_Transform.m_Position;
		ownerDiff.y() = 0.0f;
		distance = min(distance, ownerDiff.norm());
	}

	if (!m_targetLost && distance > c_TargetLostDistance)
	{
		// too far, get back into c_MaxEncirclementRadius
		m_targetLost = true;
	}

	if (m_targetLost && distance > c_MaxEncirclementRadius)
	{
		fnAccel(m_speed, c_ApproachSpeed, c_DeltaSpeed);
	}
	else if (distance < c_MinEncirclementRadius)
	{
		fnAccel(m_speed, -c_EscapeSpeed, c_DeltaSpeed);
	}
	else
	{
		fnAccel(m_speed, 0.0f, c_DeltaSpeed);
	}

	FaceDirection(m_direction);

	// update position
	m_spProxy->m_Position = m_spMatrixNodeTransform->m_Transform.m_Position;
	m_spProxy->m_UpVector = hh::math::CVector::UnitY();
	m_spProxy->m_Velocity = m_direction * m_speed;

	if (m_spMatrixNodeTransform->m_Transform.m_Position.y() + c_DodgeSpeed * dt + 0.1f < m_startPos.y())
	{
		m_spProxy->m_Velocity.y() += c_DodgeSpeed;
	}
	else if (m_spMatrixNodeTransform->m_Transform.m_Position.y() - c_DodgeSpeed * dt - 0.1f > m_startPos.y())
	{
		m_spProxy->m_Velocity.y() -= c_DodgeSpeed;
	}

	Common::fCCharacterProxyIntegrate(m_spProxy.get(), dt);
	m_spMatrixNodeTransform->m_Transform.SetPosition(m_spProxy->m_Position);
	m_spMatrixNodeTransform->NotifyChanged();
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
	SendMessage(m_owner, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(0));

	if (!m_disableDamageSfx)
	{
		SharedPtrTypeless soundHandle;
		Common::ObjectPlaySound(this, 200614000, soundHandle);
	}

	ChangeState(Dead);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Body", false);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Player", false);

	// terrain/enemy detection
	hk2010_2_0::hkpSphereShape* damageShape = new hk2010_2_0::hkpSphereShape(0.3f);
	AddEventCollision("Terrain", damageShape, *(int*)0x1E0AFA4, true, m_spNodeBody); // TypeTerrain + TypeWater

	// face opposite of blown direction
	FaceDirection(-m_direction);
	m_speed = c_BlownSpeed;

	// get blown when about to explode, remove effect
	if (m_suicideID)
	{
		m_pGlitterPlayer->StopByID(m_suicideID, true);
		m_suicideID = 0;
	}
}

void MephilesShadow::StateBlownAdvance(float dt)
{
	hh::math::CVector const newPosition = m_spMatrixNodeTransform->m_Transform.m_Position + m_direction * m_speed * dt;
	m_spMatrixNodeTransform->m_Transform.SetPosition(newPosition);
	m_spMatrixNodeTransform->NotifyChanged();

	float constexpr c_BlownTime = 0.25f;
	if (m_stateTime >= c_BlownTime || m_speed == 0.0f)
	{
		m_stateNext = State::Dead;
	}
}

void MephilesShadow::StateBlownEnd()
{
	GetKilled(true, true, !m_disableDamageSfx);
}

//---------------------------------------------------
// State::Shock
//---------------------------------------------------
void MephilesShadow::StateShockBegin()
{
	m_type = Type::Encirclement;
	m_speed = 0.0f;

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
// State::SpringWait
//---------------------------------------------------
void MephilesShadow::StateCircleWaitBegin()
{
	m_attackID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spModel->GetNode("Spine"), "ef_mephiles_attack", 1.0f);
}

void MephilesShadow::StateCircleWaitAdvance(float dt)
{
	hh::math::CVector const displacement = m_spMatrixNodeTransform->m_Transform.m_Position - m_startPos;
	hh::math::CVector const newPosition = m_startPos + Eigen::AngleAxisf(c_CircularFlightSpeed, Eigen::Vector3f::UnitY()) * displacement;
	m_spMatrixNodeTransform->m_Transform.SetPosition(newPosition);
	m_spMatrixNodeTransform->NotifyChanged();
	FaceDirection(GetPlayerDirection(true));

	if (m_stateTime >= m_attackStartTime)
	{
		switch (m_type)
		{
		case Type::Spring:	m_stateNext = State::SpringAttack;	break;
		case Type::Charge:	m_stateNext = State::ChargeAttack;	break;
		default:			m_stateNext = State::Idle;			break;
		}
	}
}

void MephilesShadow::StateCircleWaitEnd()
{
	bool const isAttacking = (m_stateNext == State::SpringAttack || m_stateNext == State::ChargeAttack);
	if (!isAttacking && m_attackID)
	{
		m_pGlitterPlayer->StopByID(m_attackID, false);
		m_attackID = 0;
	}
}

//---------------------------------------------------
// State::SpringAttack
//---------------------------------------------------
void MephilesShadow::StateSpringAttackBegin()
{
	ChangeState(Catch);

	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector const playerPos = context->m_spMatrixNode->m_Transform.m_Position;
	float const randAngle = RAND_FLOAT(0.0f, 2.0f * PI_F);
	float const randError = RAND_FLOAT(0.0f, c_SpringErrorRadius);
	m_attackTarget = playerPos + Eigen::AngleAxisf(randAngle, Eigen::Vector3f::UnitY()) * hh::math::CVector::UnitZ() * randError;
	
	hh::math::CVector dir = m_attackTarget - m_spMatrixNodeTransform->m_Transform.m_Position;
	dir.y() = 0.0f;

	float const timeToPlayer = dir.norm() / c_SpringSpeed; // horizontal only
	float const yDist = m_attackTarget.y() - m_spMatrixNodeTransform->m_Transform.m_Position.y();
	float const ySpeed = (yDist - 0.5f * c_SpringG * timeToPlayer * timeToPlayer) / timeToPlayer;

	m_direction = dir.normalized();
	m_speed = ySpeed; // for y-direction only
	FaceDirection(m_direction);

	m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, "ef_mephiles_spring", 1.0f, 1);
}

void MephilesShadow::StateSpringAttackAdvance(float dt)
{
	hh::math::CVector const oldPos = m_spMatrixNodeTransform->m_Transform.m_Position;
	hh::math::CVector newPos = oldPos + m_direction * c_SpringSpeed * dt + hh::math::CVector::UnitY() * m_speed * dt;

	// raycast terrain
	RayCastQuery rayCastQuery;
	if (Common::fRayCastQuery(*(int*)0x1E0AFAC, rayCastQuery, oldPos, newPos))
	{
		newPos = rayCastQuery.position;

		int terrainType = -1;
		if (rayCastQuery.rigidBody && Common::fRigidBodyGetIntProperty(rayCastQuery.rigidBody, 8196, terrainType) && terrainType == 19)
		{
			// hit lava, die immediately
			GetKilled(true, true, true);

			// notify owner it's dying
			SendMessage(m_owner, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(0));
		}
		else
		{
			m_stateNext = State::SpringMiss;
		}
	}

	// update gravity
	m_speed += c_SpringG * dt;

	m_spMatrixNodeTransform->m_Transform.SetPosition(newPos);
	m_spMatrixNodeTransform->NotifyChanged();
}

void MephilesShadow::StateSpringAttackEnd()
{
	if (m_attackID)
	{
		m_pGlitterPlayer->StopByID(m_attackID, false);
		m_attackID = 0;
	}
}

//---------------------------------------------------
// State::SpringMiss
//---------------------------------------------------
void MephilesShadow::StateSpringMissBegin()
{
	ChangeState(CatchMiss);
}

void MephilesShadow::StateSpringMissAdvance(float dt)
{
	if (m_stateTime >= c_SpringFailedTime)
	{
		m_enableDamageTimer = 0.5f;
		m_stateNext = State::Idle;
	}
}

//---------------------------------------------------
// State::SpringAttach
//---------------------------------------------------
void MephilesShadow::StateSpringAttachBegin()
{
	// due to message delay, it's possible the previous state was SpringMiss and changed animation
	if (GetCurrentState()->GetStateName() != Catch)
	{
		ChangeState(Catch);
		m_spAnimPose->Update(0.5f);
	}

	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Body", false);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Player", false);

	// set rotation relative to player's rotation
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CQuaternion const newRotation = context->m_spMatrixNode->m_Transform.m_Rotation.conjugate() * m_spNodeModel->m_Transform.m_Rotation;

	float constexpr c_HoldSpace = 0.2f;
	hh::math::CVector const offset = newRotation * hh::math::CVector::UnitZ() * c_HoldSpace;
	m_spNodeModel->m_Transform.SetRotationAndPosition(newRotation, -offset);
	m_spNodeModel->NotifyChanged();
}

void MephilesShadow::StateSpringAttachAdvance(float dt)
{
	if (S06DE_API::GetChaosBoostLevel() > 0)
	{
		// shift offset position back to root, move up a little to prevent colliding with terrain
		hh::math::CVector const actualPosition = m_spNodeModel->GetWorldMatrix().translation();
		m_spNodeModel->m_Transform.SetPosition(hh::math::CVector::Zero());
		m_spNodeModel->NotifyChanged();
		m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(hh::math::CQuaternion::Identity(), actualPosition + hh::math::CVector::UnitY() * 0.2f);
		m_spMatrixNodeTransform->NotifyChanged();

		m_disableDamageSfx = true;
		m_direction = -GetPlayerDirection(false);
		m_stateNext = State::Blown;
		return;
	}

	// attach to player, after player has updated position
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::mr::CTransform const playerTrans = context->m_spMatrixNode->m_Transform;
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(playerTrans.m_Rotation, playerTrans.m_Position);
	m_spMatrixNodeTransform->NotifyChanged();
}

//---------------------------------------------------
// State::ChargeAttack
//---------------------------------------------------
void MephilesShadow::StateChargeAttackBegin()
{
	ChangeState(BodyAttack);

	SharedPtrTypeless soundHandle;
	Common::ObjectPlaySound(this, 200615004, soundHandle);

	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector const playerPos = context->m_spMatrixNode->m_Transform.m_Position + hh::math::CVector::UnitY() * 0.1f;
	float const randAngle = RAND_FLOAT(0.0f, 2.0f * PI_F);
	float const randError = RAND_FLOAT(0.0f, c_ChargeErrorRadius);
	m_attackTarget = playerPos + Eigen::AngleAxisf(randAngle, Eigen::Vector3f::UnitY()) * hh::math::CVector::UnitZ() * randError;
	m_attackStart = m_spMatrixNodeTransform->m_Transform.m_Position;

	hh::math::CVector dir = m_attackTarget - m_attackStart;
	dir.y() = 0.0f;

	m_direction = dir.normalized();
	FaceDirection(m_direction);
}

void MephilesShadow::StateChargeAttackAdvance(float dt)
{
	hh::math::CVector newPosition = m_spMatrixNodeTransform->m_Transform.m_Position + m_direction * c_ChargeSpeed * dt;
	hh::math::CVector newDist = m_attackTarget - m_spMatrixNodeTransform->m_Transform.m_Position;
	hh::math::CVector totalDist = m_attackTarget - m_attackStart;
	newDist.y() = 0.0f;
	totalDist.y() = 0.0f;

	float const signedDist = newDist.norm() * (newDist.dot(m_direction) < 0.0f ? -1.0f : 1.0f);
	float const newDistProp = signedDist / totalDist.norm();
	float const totalYDist = m_attackTarget.y() - m_attackStart.y();
	newPosition.y() = m_attackTarget.y() - newDistProp * newDistProp * totalYDist;

	hh::math::CVector const newDirection = (newPosition - m_spMatrixNodeTransform->m_Transform.m_Position).normalized();
	hh::math::CVector dirXZ = newDirection; dirXZ.y() = 0.0f; dirXZ.normalize();
	hh::math::CQuaternion const rotYaw = hh::math::CQuaternion::FromTwoVectors(hh::math::CVector::UnitZ(), m_direction.head<3>());
	hh::math::CQuaternion const rotPitch = hh::math::CQuaternion::FromTwoVectors(dirXZ.head<3>(), newDirection.head<3>());

	if (m_stateTime < c_ChargeStartTime)
	{
		hh::math::CQuaternion const newRotation = m_spNodeModel->m_Transform.m_Rotation.slerp(m_stateTime / c_ChargeStartTime, rotPitch * rotYaw);
		m_spNodeModel->m_Transform.SetRotation(newRotation);
		m_spNodeModel->NotifyChanged();
	}
	else
	{
		m_spNodeModel->m_Transform.SetRotation(rotPitch * rotYaw);
		m_spNodeModel->NotifyChanged();

		m_spMatrixNodeTransform->m_Transform.SetPosition(newPosition);
		m_spMatrixNodeTransform->NotifyChanged();
	}

	if (signedDist / c_ChargeSpeed <= -1.0f 
	|| (totalYDist < 0.0f && newPosition.y() > m_attackStart.y() + 0.1f)
	|| (totalYDist >= 0.0f && newPosition.y() < m_groundHeight))
	{
		m_stateNext = State::Idle;
	}
}

void MephilesShadow::StateChargeAttackEnd()
{
	if (m_attackID)
	{
		m_pGlitterPlayer->StopByID(m_attackID, false);
		m_attackID = 0;
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
	return m_type != Type::Spring && m_enableDamageTimer <= 0.0f;
}

void MephilesShadow::PlayHitEffect()
{
	// play hit pfx here, otherwise it can only play from player, not shock waves
	auto attackEffectNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
	attackEffectNode->SetParent(Sonic::CApplicationDocument::GetInstance()->m_pMember->m_spMatrixNodeRoot.get());
	attackEffectNode->m_Transform.SetPosition(GetBodyPosition());
	attackEffectNode->NotifyChanged();
	m_pGlitterPlayer->PlayOneshot(attackEffectNode, "ef_mephiles_attack_hit", 1.0f, 1);
}

void MephilesShadow::GetKilled(bool spawnParticle, bool spawnBoostEnergy, bool playSfx)
{
	if (spawnParticle)
	{
		m_pGlitterPlayer->PlayOneshot(m_spModel->GetNode("Spine"), "ef_mephiles_dead", 1.0f, 1);
	}

	if (spawnBoostEnergy)
	{
		Common::SpawnBoostParticle((uint32_t**)this, m_spMatrixNodeTransform->m_Transform.m_Position, 0x10001);
	}

	if (playSfx)
	{
		SharedPtrTypeless soundHandle;
		Common::ObjectPlaySound(this, 200615001, soundHandle);
	}

	Kill();
}

void MephilesShadow::FaceDirection(hh::math::CVector dir)
{
	if (dir.y() != 0.0f)
	{
		dir.y() = 0.0f;
		dir.normalize();
	}

	hh::math::CQuaternion const rotation = hh::math::CQuaternion::FromTwoVectors(hh::math::CVector::UnitZ(), dir.head<3>());
	m_spNodeModel->m_Transform.SetRotation(rotation);
	m_spNodeModel->NotifyChanged();
}

hh::math::CVector MephilesShadow::GetPlayerDirection(bool zeroY, float* distance) const
{
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector const playerPos = context->m_spMatrixNode->m_Transform.m_Position;
	hh::math::CVector dir = playerPos - m_spMatrixNodeTransform->m_Transform.m_Position;
	if (distance) *distance = dir.norm();
	if (zeroY) dir.y() = 0.0f;
	dir.normalize();
	return dir;
}
