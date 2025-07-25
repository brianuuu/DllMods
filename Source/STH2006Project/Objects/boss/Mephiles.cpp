#include "Mephiles.h"

#include "Managers/MstManager.h"
#include "Objects/enemy/EnemyShield.h"
#include "UI/LoadingUI.h"
#include "UI/SubtitleUI.h"

int Mephiles::m_encounterCount = 0;
HOOK(int, __fastcall, Mephiles_MsgRestartStage, 0xE76810, uint32_t* This, void* Edx, Sonic::Message::MsgRestartStage& message)
{
	if (message.m_Flag == 0)
	{
		Mephiles::m_encounterCount = 0;
	}
	return originalMephiles_MsgRestartStage(This, Edx, message);
}

BB_SET_OBJECT_MAKE_HOOK(Mephiles)
void Mephiles::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(Mephiles);
	applyPatches();
}

void Mephiles::applyPatches()
{
	INSTALL_HOOK(Mephiles_MsgRestartStage);
}

void Mephiles::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	in_rEditParam.CreateParamInt(&m_Data.m_MaxHP, "MaxHP");
	in_rEditParam.CreateParamFloat(&m_Data.m_GroundHeight, "GroundHeight");
	in_rEditParam.CreateParamBase(Sonic::CParamPosition::Create(&m_Data.m_SunDirection), "SunDirection");
}

char const* Mephiles::HideLoop = "HideLoop";
char const* Mephiles::HideCommand = "HideCommand";
char const* Mephiles::Suffer = "Suffer";
char const* Mephiles::Wait = "Wait";

bool Mephiles::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	char const* modelName = "en_fmef_Root";
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(modelName, 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, true);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, modelName);
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo(HideLoop, "en_shwait_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_Loop));
	entries.push_back(hh::anim::SMotionInfo(HideCommand, "en_shcommand_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo(Suffer, "en_suffer_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo(Wait, "en_wait_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_Loop));
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
	fnAddAnimationState(HideLoop);
	fnAddAnimationState(HideCommand, HideLoop);
	fnAddAnimationState(Wait);
	fnAddAnimationState(Suffer, Wait);
	SetAnimationBlend(HideLoop, HideCommand, 0.5f);
	SetAnimationBlend(HideCommand, HideLoop, 0.5f);
	SetAnimationBlend(Suffer, Wait, 0.5f);
	ChangeState(HideLoop);

	SetCullingRange(0.0f);

	return true;
}

bool Mephiles::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	auto const& bodyCenter = m_spModel->GetNode("Hips");
	m_spNodeBody = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeBody->SetParent(bodyCenter.get());

	// rigid body
	hk2010_2_0::hkpSphereShape* rigidBodyShape = new hk2010_2_0::hkpSphereShape(0.9f);
	AddEventCollision("Body", rigidBodyShape, *(int*)0x1E0AF24, true, m_spNodeBody); // CollisionTypeEnemyAndLockOn

	// damage body
	hk2010_2_0::hkpSphereShape* damageShape = new hk2010_2_0::hkpSphereShape(0.5f);
	AddEventCollision("Damage", damageShape, *(int*)0x1E0AFD8, true, m_spNodeBody); // ColID_PlayerEvent

	return true;
}

void Mephiles::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	// load required mst files
	MstManager::RequestMst("msg_hint");
	MstManager::RequestMst("msg_hint_xenon");

	// cache hide rotation
	if (m_Data.m_SunDirection != hh::math::CVector::Zero())
	{
		hh::math::CVector faceDirection = hh::math::CVector::Zero() - m_Data.m_SunDirection;
		faceDirection.y() = 0.0f;
		faceDirection.normalize();
		m_hideRotation = hh::math::CQuaternion::FromTwoVectors(hh::math::CVector::UnitZ(), faceDirection.head<3>());
	}

	StateAppearBegin();

	// disable Chaos Drive sfx
	S06DE_API::SetChaosEnergyRewardOverride(0.5f);
	WRITE_JUMP(0x11245A3, (void*)0x11245AF);
}

void Mephiles::KillCallback()
{
	// re-enable Chaos Drive sfx
	S06DE_API::SetChaosEnergyRewardOverride();
	WRITE_MEMORY(0x11245A3, uint8_t, 0xE8, 0x98, 0xA4, 0xC4, 0xFF);
}

void Mephiles::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	if (m_stateNext != m_state)
	{
		HandleStateChange();
	}

	switch (m_state)
	{
	case State::Appear: StateAppearAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::Hide: StateHideAdvance(in_rUpdateInfo.DeltaTime); break;
	}

	m_damagedThisFrame = false;
	m_stateTime += in_rUpdateInfo.DeltaTime;

	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);

	AdvanceSpawnShadow(in_rUpdateInfo.DeltaTime);
}

bool Mephiles::ProcessMessage
(
	Hedgehog::Universe::Message& message, bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgDamage>())
		{
			auto& msg = static_cast<Sonic::Message::MsgDamage&>(message);

			// put player at fixed distance from Mephiles
			hh::math::CVector const bodyBase = GetBodyPosition() - hh::math::CVector::UnitY() * 0.5f;
			hh::math::CVector const otherPos = bodyBase + (msg.m_DamagePosition - bodyBase).normalized() * 1.2f;
			SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamageSuccess>(otherPos, true));

			if (!m_damagedThisFrame)
			{
				m_damagedThisFrame = true;
				if (CanDamage())
				{
					// TODO:
				}
				else
				{
					if (SendMessageImm(message.m_SenderActorID, Sonic::Message::MsgGetPlayerType()))
					{
						CreateShield(msg.m_DamagePosition + hh::math::CVector::UnitY() * 0.5f);
					}
					else
					{
						CreateShield(msg.m_DamagePosition);
					}
				}
			}
			return true;
		}
		
		if (message.Is<Sonic::Message::MsgNotifyShockWave>())
		{
			auto& msg = static_cast<Sonic::Message::MsgNotifyShockWave&>(message);
			if (CanDamage())
			{
				// TODO:
			}
			else
			{
				CreateShield(msg.m_Position);
			}
			return true;
		}

		if (message.Is<Sonic::Message::MsgHitEventCollision>())
		{
			auto& msg = static_cast<Sonic::Message::MsgHitEventCollision&>(message);
			if (msg.m_Symbol == "Damage")
			{
				SendMessage(msg.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
					(
						*(uint32_t*)0x1E0BE34, // DamageID_NoAttack
						GetBodyPosition(),
						hh::math::CVector::Zero()
					)
				);
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
				m_numKilledUnit++; 
				m_shadows.erase(message.m_SenderActorID);
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

		if (message.Is<Sonic::Message::MsgGetHomingAttackPriority>() && CanLock())
		{
			auto& msg = static_cast<Sonic::Message::MsgGetHomingAttackPriority&>(message);
			*msg.m_pPriority = 10;
			return true;
		}

		if (message.Is<Sonic::Message::MsgGetHomingAttackPosition>() && CanLock())
		{
			auto& msg = static_cast<Sonic::Message::MsgGetHomingAttackPosition&>(message);
			*msg.m_pPosition = GetBodyPosition();
			return true;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void Mephiles::HandleStateChange()
{
	// end current state
	switch (m_state)
	{
	}

	// start next state
	switch (m_stateNext)
	{
	case State::Hide: StateHideBegin(); break;
	case State::Eject: StateEjectBegin(); break;
	}

	m_state = m_stateNext;
	m_stateStage = 0;
	m_stateTime = 0.0f;
}

//---------------------------------------------------
// State::Appear
//---------------------------------------------------
void Mephiles::StateAppearBegin()
{
	SetHidden(true);

	// Remove shadow from player
	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	auto& bundle = m_pMember->m_pWorld->m_pMember->m_BundlePairMap;
	bundle.at("SMO")->m_pBundle->RemoveRenderable(context->m_pPlayer->m_spCharacterModel);
}

void Mephiles::StateAppearAdvance(float dt)
{
	switch (m_stateStage)
	{
	case 0:
	{
		if (!LoadingUI::IsEnabled())
		{
			ChangeState(HideCommand);
			m_stateStage++;
		}
		break;
	}
	case 1:
	{
		// wait for shadow spawn to finish
		if (GetCurrentState()->GetStateName() == HideLoop)
		{
			SharedPtrTypeless soundHandle;
			Common::ObjectPlaySound(this, 200615000, soundHandle);

			SpawnEncirclement(200, 15.0f);
			m_stateTime = 0.0f;
			m_stateStage++;
		}
		break;
	}
	case 2:
	{
		if (m_stateTime >= 1.5f)
		{
			m_stateNext = State::Hide;
		}
		break;
	}
	}

	// initial VO
	if (!m_playedInitVO && !LoadingUI::IsEnabled())
	{
		m_playedInitVO = true;
		if (m_encounterCount < 3 && S06DE_API::GetModelType() == S06DE_API::ModelType::Shadow)
		{
			switch (m_encounterCount)
			{
			case 0: PlaySingleVO("msg_hint", "hint_bos04_e00_sd"); break;
			case 1: PlaySingleVO("msg_hint_xenon", "hint_bos04_a08_sd"); break;
			case 2: PlaySingleVO("msg_hint", "hint_bos04_a02_sd"); break;
			}
		}
		m_encounterCount++;
	}

	FollowPlayer();
}

//---------------------------------------------------
// State::Hide
//---------------------------------------------------
void Mephiles::StateHideBegin()
{
	SetHidden(true);
}

void Mephiles::StateHideAdvance(float dt)
{
	switch (m_stateStage)
	{
	case 0:
	{
		if (m_stateTime >= 1.5f)
		{
			if (m_numKilledUnit >= 90)
			{
				PlaySingleVO("msg_hint", "hint_bos04_e03_mf");
			}
			else if (m_numKilledUnit >= 60)
			{
				PlaySingleVO("msg_hint", "hint_bos04_a00_sd");
			}
			else if (m_numKilledUnit >= 30)
			{
				PlaySingleVO("msg_hint", "hint_bos04_e02_mf");
			}

			ChangeState(HideCommand);
			m_stateStage++;
		}
		break;
	}
	case 1:
	case 3:
	{
		if (GetCurrentState()->GetStateName() == HideLoop)
		{
			SharedPtrTypeless soundHandle;
			Common::ObjectPlaySound(this, 200615000, soundHandle);

			m_stateTime = 0.0f;
			if (m_shadows.size() < 100 || m_stateStage == 3)
			{
				// too few shadows
				SpawnEncirclement(50, (m_stateStage == 3) ? 25.0f : 20.0f);
				m_stateStage++;
			}
			else
			{
				float constexpr MinAppearRadius = 5.0f;
				float constexpr MaxAppearRadius = 7.5f;
				float const radius = RAND_FLOAT(MinAppearRadius, MaxAppearRadius);
				if (m_numKilledUnit >= 90)
				{
					SpawnSpring(50, radius, 1.0f, 2.0f);
				}
				else if (m_numKilledUnit >= 60)
				{
					SpawnSpring(40, radius, 1.5f, 1.5f);
				}
				else if (m_numKilledUnit >= 30)
				{
					SpawnSpring(30, radius, 2.0f, 1.0f);
				}
				else
				{
					SpawnSpring(20, radius, 2.5f, 0.5f);
				}
				m_stateStage = 5;
			}
		}
		break;
	}
	case 2:
	case 4:
	{
		if (m_stateTime >= 1.5f)
		{
			if (m_stateStage == 4)
			{
				// restart state
				m_stateTime = 0.0f;
				m_stateStage = 0;
			}
			else
			{
				// spawn more encirclement shadows
				ChangeState(HideCommand);
				m_stateStage++;
			}
		}
		break;
	}
	case 5:
	{
		if (m_stateTime >= 8.0f)
		{
			// restart state
			m_stateTime = 0.0f;
			m_stateStage = 0;
		}
		break;
	}
	}

	FollowPlayer();
}

//---------------------------------------------------
// State::Eject
//---------------------------------------------------
void Mephiles::StateEjectBegin()
{
	ChangeState(Suffer);
	SetHidden(false);

	// copy player's transform
	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	m_spMatrixNodeTransform->m_Transform = context->m_spMatrixNode->m_Transform;
	m_spMatrixNodeTransform->NotifyChanged();
}

//---------------------------------------------------
// Utils
//---------------------------------------------------
hh::math::CVector Mephiles::GetBodyPosition() const
{
	return m_spModel->GetNode("Hips")->GetWorldMatrix().translation();
}

bool Mephiles::CanLock() const
{
	return m_state != State::Hide;
}

bool Mephiles::CanDamage() const
{
	return false;
}

void Mephiles::CreateShield(hh::math::CVector const& otherPos) const
{
	hh::mr::CTransform startTrans{};
	hh::math::CVector const bodyCenter = GetBodyPosition();
	hh::math::CVector const dir = (otherPos - bodyCenter).normalized();
	if (dir.dot(hh::math::CVector::UnitY()) <= 0.95f) // not pointing up
	{
		hh::math::CVector hDir = dir;
		hDir.y() = 0.0f;
		hDir.normalize();
		float yaw = acos(hDir.z());
		if (hDir.dot(Eigen::Vector3f::UnitX()) < 0) yaw = -yaw;
		startTrans.m_Rotation = hh::math::CQuaternion::FromTwoVectors(hDir.head<3>(), dir.head<3>()) * Eigen::AngleAxisf(yaw, Eigen::Vector3f::UnitY());
	}
	else
	{
		startTrans.m_Rotation = hh::math::CQuaternion::FromTwoVectors(hh::math::CVector::UnitZ(), dir.head<3>());
	}

	startTrans.m_Position = bodyCenter + dir * 0.7f;
	m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<EnemyShield>(startTrans, true));
}

void Mephiles::PlaySingleVO(std::string const& name, std::string const& id)
{
	if (m_playedVO.count(id) || S06DE_API::GetModelType() != S06DE_API::ModelType::Shadow) return;
	m_playedVO.insert(id);

	SubtitleUI::addSubtitle(name, id);
}

void Mephiles::SetHidden(bool hidden)
{
	if (hidden && !m_isHidden)
	{
		// remove model but keep shadow map
		auto& bundle = m_pMember->m_pWorld->m_pMember->m_BundlePairMap;
		bundle.at("Object")->m_pBundle->RemoveRenderable(m_spModel);
		m_spAnimPose->m_Scale = 3.0f;

		// face towards Sun
		m_spMatrixNodeTransform->m_Transform.SetRotation(m_hideRotation);
		m_spMatrixNodeTransform->NotifyChanged();

		// disable collision
		Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Body", false);
		Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Damage", false);

		m_isHidden = true;
	}
	else if (!hidden && m_isHidden)
	{
		// add model rendering back
		Sonic::CGameObject::AddRenderable("Object", m_spModel, false);
		m_spAnimPose->m_Scale = 1.0f;

		// re-enable collision
		Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Body", true);
		Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Damage", true);

		m_isHidden = false;
	}
}

void Mephiles::FollowPlayer()
{
	if (S06DE_API::GetChaosBoostLevel() > 0)
	{
		SubtitleUI::addSubtitle("msg_hint", m_hasEjected ? "hint_bos04_e12_mf" : "hint_bos04_e06_mf");

		m_stateNext = State::Eject;
		m_hasEjected = true;
		return;
	}

	// follow player's horizontal position
	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector pos = context->m_spMatrixNode->m_Transform.m_Position;
	pos.y() = m_Data.m_GroundHeight;
	m_spMatrixNodeTransform->m_Transform.SetPosition(pos);
	m_spMatrixNodeTransform->NotifyChanged();
}

//---------------------------------------------------
// Shadow Management
//---------------------------------------------------
void Mephiles::SpawnEncirclement(int count, float radius)
{
	m_maxSpawnCount = count;
	m_spawnCount = 0;
	m_spawnTimer = 0.0f;
	m_spawnRadius = radius;
	m_spawnType = MephilesShadow::Type::Encirclement;
}

void Mephiles::SpawnSpring(int count, float radius, float attackStartTime, float attackMaxDelay)
{
	m_maxSpawnCount = count;
	m_spawnCount = 0;
	m_spawnTimer = 0.0f;
	m_spawnRadius = radius;
	m_attackStartTime = attackStartTime;
	m_attackMaxDelay = attackMaxDelay;
	m_spawnType = MephilesShadow::Type::Spring;
}

void Mephiles::AdvanceSpawnShadow(float dt)
{
	if (m_spawnCount >= m_maxSpawnCount) return;

	float constexpr spawnRate = 1.0f / 120.0f;
	m_spawnTimer += dt;
	if (m_spawnTimer >= spawnRate)
	{
		while (m_spawnCount < m_maxSpawnCount && m_spawnTimer >= spawnRate)
		{
			// max unit cap
			int constexpr MaxUnits = 256;
			if (m_shadows.size() > MaxUnits)
			{
				auto iter = m_shadows.begin();
				std::advance(iter, rand() % m_shadows.size());

				auto const& spShadowToRemove = iter->second;
				SendMessageImm(spShadowToRemove->m_ActorID, Sonic::Message::MsgNotifyObjectEvent(0));

				m_numKilledUnit++; // 06 counts removed units as killed
				m_shadows.erase(iter);
			}

			float angle = 0.0f;
			switch (m_spawnType)
			{
			case MephilesShadow::Type::Encirclement: 
			{
				angle = RAND_FLOAT(0.0f, 2.0f * PI_F); 
				break;
			}
			case MephilesShadow::Type::Spring:
			{
				angle = 0.01f + 2.0f * PI_F * (float)m_spawnCount / (float)m_maxSpawnCount;
				break;
			}
			}

			auto spShadow = boost::make_shared<MephilesShadow>(m_ActorID, m_spawnType, m_spawnRadius, angle, GetShadowSpawnPosition());
			m_pMember->m_pGameDocument->AddGameObject(spShadow);
			m_shadows[spShadow->m_ActorID] = spShadow;

			if (m_spawnType == MephilesShadow::Type::Spring)
			{
				spShadow->SetInitialStateSpring(m_attackStartTime, m_attackMaxDelay);
			}
			
			m_spawnCount++;
			m_spawnTimer -= spawnRate;
		}
	}
}

hh::math::CVector Mephiles::GetShadowSpawnPosition() const
{
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector const playerPos = context->m_spMatrixNode->m_Transform.m_Position;

	switch (m_spawnType)
	{
	case MephilesShadow::Type::Encirclement:
	case MephilesShadow::Type::Spring:
	{
		hh::math::CVector spawnPos = playerPos;
		spawnPos.y() = m_Data.m_GroundHeight;
		return spawnPos;
	}
	}

	return hh::math::CVector::Zero();
}
