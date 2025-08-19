#include "Mephiles.h"

#include "Managers/MstManager.h"
#include "Managers/ScoreManager.h"
#include "Objects/enemy/DarkSphere.h"
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

Mephiles::~Mephiles()
{
	if (m_Data.m_PositionList)
	{
		m_Data.m_PositionList->Release();
	}
}

void Mephiles::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	in_rEditParam.CreateParamInt(&m_Data.m_MaxHP, "MaxHP");
	in_rEditParam.CreateParamFloat(&m_Data.m_GroundHeight, "GroundHeight");
	in_rEditParam.CreateParamBase(Sonic::CParamPosition::Create(&m_Data.m_SunDirection), "SunDirection");

	char const* positionList = "PositionList";
	m_Data.m_PositionList = Sonic::CParamTargetList::Create(positionList);
	if (m_Data.m_PositionList)
	{
		m_Data.m_PositionList->AddRef();
	}
	in_rEditParam.CreateParamBase(m_Data.m_PositionList, positionList);

	in_rEditParam.CreateParamBase(Sonic::CParamTarget::Create(&m_Data.m_CameraLock), "CameraLock");
	in_rEditParam.CreateParamBase(Sonic::CParamTarget::Create(&m_Data.m_CameraLockDive), "CameraLockDive");
	in_rEditParam.CreateParamBase(Sonic::CParamTarget::Create(&m_Data.m_CameraPan), "CameraPan");
	in_rEditParam.CreateParamBase(Sonic::CParamTarget::Create(&m_Data.m_CameraPanNoEase), "CameraPanNoEase");
	in_rEditParam.CreateParamBase(Sonic::CParamTarget::Create(&m_Data.m_FocusObject), "FocusObject");
}

char const* Mephiles::HideLoop = "HideLoop";
char const* Mephiles::HideCommand = "HideCommand";
char const* Mephiles::Command = "Command";
char const* Mephiles::Dive = "Dive";
char const* Mephiles::Grin = "Grin";
char const* Mephiles::GrinLoop = "GrinLoop";
char const* Mephiles::Shock = "Shock";
char const* Mephiles::Smile = "Smile";
char const* Mephiles::Suffer = "Suffer";
char const* Mephiles::Tired = "Tired";
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
	entries.push_back(hh::anim::SMotionInfo(Command, "en_command_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo(Dive, "en_dive_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo(Grin, "en_grin_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo(GrinLoop, "en_grin_l_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_Loop));
	entries.push_back(hh::anim::SMotionInfo(Shock, "en_shock_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo(Smile, "en_smile_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_Loop));
	entries.push_back(hh::anim::SMotionInfo(Suffer, "en_suffer_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo(Tired, "en_tired_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_Loop));
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
	fnAddAnimationState(Command, Wait);
	fnAddAnimationState(Dive);
	fnAddAnimationState(GrinLoop);
	fnAddAnimationState(Grin, GrinLoop);
	fnAddAnimationState(Shock, Wait);
	fnAddAnimationState(Smile);
	fnAddAnimationState(Suffer, Wait);
	fnAddAnimationState(Tired);
	SetAnimationBlend(HideLoop, HideCommand, 0.5f);
	SetAnimationBlend(HideCommand, HideLoop, 0.5f);
	SetAnimationBlend(Dive, HideLoop, 0.5f);
	SetAnimationBlend(Command, Dive, 0.5f);
	SetAnimationBlend(GrinLoop, Dive, 0.5f);
	SetAnimationBlend(Grin, Dive, 0.5f);
	SetAnimationBlend(Shock, Dive, 0.5f);
	SetAnimationBlend(Smile, Dive, 0.5f);
	SetAnimationBlend(Wait, Dive, 0.5f);
	SetAnimationBlend(Wait, Command, 0.5f);
	SetAnimationBlend(Command, Wait, 0.5f);
	SetAnimationBlend(Wait, Grin, 0.5f);
	SetAnimationBlend(GrinLoop, Wait, 0.5f);
	SetAnimationBlend(Grin, Wait, 0.5f);
	SetAnimationBlend(Shock, Wait, 0.5f);
	SetAnimationBlend(Wait, Smile, 0.5f);
	SetAnimationBlend(Smile, Wait, 0.5f);
	SetAnimationBlend(Suffer, Wait, 0.5f);
	SetAnimationBlend(Tired, Wait, 0.5f);
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
	hk2010_2_0::hkpSphereShape* rigidBodyShape = new hk2010_2_0::hkpSphereShape(0.8f);
	AddEventCollision("Body", rigidBodyShape, *(int*)0x1E0AF24, true, m_spNodeBody); // CollisionTypeEnemyAndLockOn

	// damage body
	hk2010_2_0::hkpSphereShape* damageShape = new hk2010_2_0::hkpSphereShape(0.5f);
	AddEventCollision("Damage", damageShape, *(int*)0x1E0AFD8, true, m_spNodeBody); // ColID_PlayerEvent

	// barrier
	hk2010_2_0::hkpSphereShape* barrierShape = new hk2010_2_0::hkpSphereShape(1.5f);
	AddEventCollision("Barrier", barrierShape, *(int*)0x1E0AFD8, true, m_spNodeBody); // ColID_PlayerEvent
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Barrier", false);

	// dive shockwave
	hk2010_2_0::hkpSphereShape* shockShape = new hk2010_2_0::hkpSphereShape(6.0f);
	AddEventCollision("Shock", shockShape, *(int*)0x1E0AFD8, true, m_spMatrixNodeTransform); // ColID_PlayerEvent
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Shock", false);

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

	m_HP = m_Data.m_MaxHP;
	S06HUD_API::SetBossHealth(m_HP, m_Data.m_MaxHP);

	StateAppearBegin();

	// disable Chaos Drive sfx
	S06DE_API::SetChaosEnergyRewardOverride(0.5f);
	WRITE_JUMP(0x11245A3, (void*)0x11245AF);
}

void Mephiles::KillCallback()
{
	// re-enable Chaos Drive sfx
	S06DE_API::SetChaosAttackForced(false);
	S06DE_API::SetChaosEnergyRewardOverride();
	WRITE_MEMORY(0x11245A3, uint8_t, 0xE8, 0x98, 0xA4, 0xC4, 0xFF);

	ToggleSlowTime(false);
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
	case State::Eject: StateEjectAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::Dive: StateDiveAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::Warp: StateWarpAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::Damage: StateDamageAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::HalfHP: StateHalfHPAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::Dead: StateDeadAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::AttackSphereS: StateAttackSphereSAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::AttackSphereL: StateAttackSphereLAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::AttackCharge: StateAttackChargeAdvance(in_rUpdateInfo.DeltaTime); break;
	}

	m_damagedThisFrame = false;
	m_stateTime += in_rUpdateInfo.DeltaTime;

	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);

	AdvanceShadowSpawn(in_rUpdateInfo.DeltaTime);
	AdvanceShadowExplode(in_rUpdateInfo.DeltaTime);

	// spawn barrier
	if (m_spawnBarrierTimer > 0.0f)
	{
		m_spawnBarrierTimer -= in_rUpdateInfo.DeltaTime;
		if (m_spawnBarrierTimer <= 0.0f)
		{
			ToggleBarrier(true);
		}
	}
}

void Mephiles::HandleStateChange()
{
	// end current state
	switch (m_state)
	{
	case State::HalfHP: StateHalfHPEnd(); break;
	case State::AttackSphereS: StateAttackSphereSEnd(); break;
	case State::AttackSphereL: StateAttackSphereLEnd(); break;
	case State::AttackCharge: StateAttackChargeEnd(); break;
	}

	// start next state
	switch (m_stateNext)
	{
	case State::Hide: StateHideBegin(); break;
	case State::Eject: StateEjectBegin(); break;
	case State::Dive: StateDiveBegin(); break;
	case State::Warp: StateWarpBegin(); break;
	case State::Damage: StateDamageBegin(); break;
	case State::HalfHP: StateHalfHPBegin(); break;
	case State::Dead: StateDeadBegin(); break;
	case State::AttackSphereS: StateAttackSphereSBegin(); break;
	case State::AttackSphereL: StateAttackSphereLBegin(); break;
	case State::AttackCharge: StateAttackChargeBegin(); break;
	}

	m_state = m_stateNext;
	m_stateStage = 0;
	m_stateTime = 0.0f;
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

			if (!m_damagedThisFrame)
			{
				bool const isPlayer = SendMessageImm(message.m_SenderActorID, Sonic::Message::MsgGetPlayerType());
				float damageDist = 1.2f; // default shielded

				m_damagedThisFrame = true;
				if (m_barrierID)
				{
					// nothing
				}
				else if (m_canDamage)
				{
					float const prop = GetHPRatio();
					if (m_HP > 0)
					{
						if (!isPlayer)
						{
							// damage from non-player attacks
							SharedPtrTypeless soundHandle;
							Common::ObjectPlaySound(this, 200614000, soundHandle);
						}

						m_HP--;
						S06HUD_API::SetBossHealth(m_HP, m_Data.m_MaxHP);

						if (prop > 0.5f && GetHPRatio() <= 0.5f)
						{
							m_enterHalfHP = true;
						}

						m_playDamageVO = true;
						m_stateNext = (m_HP == 0) ? State::Dead : State::Damage;

						if (m_HP == 1)
						{
							m_enterLastHP = true;
							m_playDamageVO = false;
						}

						damageDist = (m_HP == 0) ? 0.4f : 0.6f;
					}
				}
				else
				{
					if (isPlayer)
					{
						CreateShield(msg.m_DamagePosition + hh::math::CVector::UnitY() * 0.5f);
					}
					else
					{
						CreateShield(msg.m_DamagePosition);
					}
				}

				// put player at fixed distance from Mephiles
				hh::math::CVector const bodyBase = GetBodyPosition() - hh::math::CVector::UnitY() * 0.5f;
				hh::math::CVector const otherPos = bodyBase + (msg.m_DamagePosition - bodyBase).normalized() * damageDist;
				SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamageSuccess>(otherPos, true));
			}
			return true;
		}
		
		if (message.Is<Sonic::Message::MsgNotifyShockWave>())
		{
			auto& msg = static_cast<Sonic::Message::MsgNotifyShockWave&>(message);
			if (m_barrierID)
			{
				ToggleBarrier(false);
			}
			else if (m_canDamage)
			{
				if (m_HP > 0)
				{
					SharedPtrTypeless soundHandle;
					Common::ObjectPlaySound(this, 200614000, soundHandle);

					m_playDamageVO = false;
					m_stateNext = State::Damage;
				}
			}
			else
			{
				CreateShield(msg.m_Position);
			}
			return true;
		}

		if (flag && message.Is<Sonic::Message::MsgIsReceiveDamage>())
		{
			auto& msg = static_cast<Sonic::Message::MsgIsReceiveDamage&>(message);
			*msg.m_pSuccess = (m_barrierID == 0);
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
			else if (msg.m_Symbol == "Barrier" || msg.m_Symbol == "Shock")
			{
				SendMessage(msg.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
					(
						*(uint32_t*)0x1E0BE28, // DamageID_SonicHeavy
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
				// a shadow has died
				m_numKilledUnit++; 
				m_shadows.erase(message.m_SenderActorID);
				m_shadowsAttached.erase(message.m_SenderActorID);
				break;
			}
			case 1:
			{
				int constexpr c_MaxHoldUnits = 5;
				if (m_shadowsAttached.size() < c_MaxHoldUnits && !m_shadowsAttached.count(message.m_SenderActorID))
				{
					// notify can attach to player
					auto& spShadow = m_shadows[message.m_SenderActorID];
					m_shadowsAttached[message.m_SenderActorID] = spShadow;
					m_shadows.erase(message.m_SenderActorID);
					SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(1));
				
					// countdown to explode
					if (m_shadowsAttached.size() == c_MaxHoldUnits)
					{
						// play countdown sfx/pfx on player
						float constexpr c_HoldExplosionWaitTime = 2.0f;
						m_attachCountdown = c_HoldExplosionWaitTime;
						m_attachSfx.reset();
						Common::SonicContextPlaySound(m_attachSfx, 200615022, 0);

						// notify playing pfx
						for (auto& iter : m_shadowsAttached)
						{
							SendMessage(iter.first, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(2));
						}
					}
				}
				break;
			}
			case 2:
			{
				// darksphere destroyed
				if (m_darkSphereL && m_darkSphereL->m_ActorID == message.m_SenderActorID)
				{
					m_darkSphereL.reset();
				}
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
	m_canDamage = false;
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
					SpawnCircleWait(50, radius, 1.0f, 2.0f, MephilesShadow::Type::Spring);
				}
				else if (m_numKilledUnit >= 60)
				{
					SpawnCircleWait(40, radius, 1.5f, 1.5f, MephilesShadow::Type::Spring);
				}
				else if (m_numKilledUnit >= 30)
				{
					SpawnCircleWait(30, radius, 2.0f, 1.0f, MephilesShadow::Type::Spring);
				}
				else
				{
					SpawnCircleWait(20, radius, 2.5f, 0.5f, MephilesShadow::Type::Spring);
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
	m_canDamage = false;

	ChangeState(Suffer);
	SetHidden(false);

	// copy player's transform
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	m_spMatrixNodeTransform->m_Transform = context->m_spMatrixNode->m_Transform;
	m_spMatrixNodeTransform->NotifyChanged();

	// enable camera
	if (!m_cameraActorID && m_Data.m_CameraPan)
	{
		m_cameraActorID = m_Data.m_CameraPan;

		hh::math::CVector const offset = hh::math::CVector(-3.5f, 1.2f, 1.8f);
		hh::math::CVector const position = context->m_spMatrixNode->m_Transform.m_Position + context->m_spMatrixNode->m_Transform.m_Rotation * offset;
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgSetPosition>(position));
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
		SetFocusCameraPosition(GetBodyPosition());
	}
}

void Mephiles::StateEjectAdvance(float dt)
{
	// disable camera
	SetFocusCameraPosition(GetBodyPosition());
	if (m_cameraActorID && m_stateTime >= 1.0f)
	{
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		m_cameraActorID = 0;
	}

	if (GetCurrentState()->GetStateName() == Wait)
	{
		m_stateNext = State::Warp;
	}
}

//---------------------------------------------------
// State::Dive
//---------------------------------------------------
void Mephiles::StateDiveBegin()
{
	m_canDamage = false;

	ChangeState(Dive);
	SubtitleUI::addSubtitle("msg_hint", "hint_bos04_e04_mf");

	if (!m_cameraActorID && m_Data.m_CameraLockDive)
	{
		m_cameraActorID = m_Data.m_CameraLockDive;
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	}
}

void Mephiles::StateDiveAdvance(float dt)
{
	switch (m_stateStage)
	{
	case 0:
	{
		TurnTowardsPlayer(dt);
		if (Common::IsAnimationFinished(this))
		{
			m_stateTime = 0.0f;
			m_stateStage++;

			auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
			m_targetPos = context->m_spMatrixNode->m_Transform.m_Position;
			m_targetPos.y() = m_Data.m_GroundHeight;
			m_warpPos = m_spMatrixNodeTransform->m_Transform.m_Position;

			// check for terrain height
			hh::math::CVector outPos = hh::math::CVector::Zero();
			hh::math::CVector outNormal = hh::math::CVector::UnitY();
			if (Common::fRaycast(m_targetPos + hh::math::CVector::UnitY(), m_targetPos - hh::math::CVector::UnitY() * 2.0f, outPos, outNormal, *(int*)0x1E0AFAC))
			{
				m_targetPos.y() = outPos.y();
			}

			hh::math::CVector dir = m_targetPos - m_warpPos;
			dir.y() = 0.0f; dir.normalize();
			m_diveYaw = acos(dir.z());
			if (dir.dot(Eigen::Vector3f::UnitX()) < 0) m_diveYaw = -m_diveYaw;

			// trail
			m_pGlitterPlayer->PlayOneshot(m_spModel->GetNode("Hips"), "ef_mephiles_warp", 1.0f, 1);
		}
		break;
	}
	case 1:
	{
		hh::math::CVector const oldPosition = m_spMatrixNodeTransform->m_Transform.m_Position;

		float constexpr c_DiveTime = 1.5f;
		float const prop = min(m_stateTime / c_DiveTime, 1.0f);
		hh::math::CVector newPosition = m_warpPos + (m_targetPos - m_warpPos) * prop;

		float constexpr initSpeed = 20.0f;
		float constexpr accel = initSpeed * -2.0f / c_DiveTime;
		float const yOffset = initSpeed * m_stateTime + 0.5f * accel * m_stateTime * m_stateTime;
		newPosition.y() += yOffset;

		float constexpr c_SpinRate = 1080.0f * DEG_TO_RAD;
		m_diveYaw += c_SpinRate * dt;

		hh::math::CQuaternion const upRotation = hh::math::CQuaternion::FromTwoVectors(hh::math::CVector::UnitY(), (newPosition - oldPosition).normalized().head<3>());
		hh::math::CQuaternion const yawRotation = Eigen::AngleAxisf(m_diveYaw, hh::math::CVector::UnitY()) * hh::math::CQuaternion::Identity();

		m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(upRotation * yawRotation, newPosition);
		m_spMatrixNodeTransform->NotifyChanged();

		if (m_stateTime >= c_DiveTime)
		{
			m_stateTime = 0.0f;
			m_stateStage++;

			SharedPtrTypeless handle;
			Common::SonicContextPlaySound(handle, 200614018, 0);

			ChangeState(HideLoop);
			ToggleBarrier(false);

			// pfx
			auto attackEffectNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
			attackEffectNode->SetParent(Sonic::CApplicationDocument::GetInstance()->m_pMember->m_spMatrixNodeRoot.get());
			attackEffectNode->m_Transform.SetPosition(m_targetPos);
			attackEffectNode->NotifyChanged();
			m_pGlitterPlayer->PlayOneshot(attackEffectNode, "ef_mephiles_dive", 1.0f, 1);
			Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Shock", true);
		}
		break;
	}
	case 2:
	{
		if (m_stateTime >= 0.5f)
		{
			if (m_cameraActorID)
			{
				Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
				m_cameraActorID = 0;
			}

			Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Shock", false);
			m_stateNext = State::Hide;
		}
		break;
	}
	}

	if (m_stateTime >= 0.5f || m_stateStage > 0)
	{
		HandleDisableCameraLock();
	}
}

//---------------------------------------------------
// State::Warp
//---------------------------------------------------
void Mephiles::StateWarpBegin()
{
	m_canDamage = false;

	if (GetCurrentState()->GetStateName() != Wait)
	{
		ChangeState(Wait);
	}

	SharedPtrTypeless handle;
	Common::SonicContextPlaySound(handle, 200615006, 0);

	if (m_enterHalfHP && m_warpIndex != 0)
	{
		// try warping to center
		m_warpIndex = 0;
	}
	else
	{
		// choose random warp location
		int size = m_Data.m_PositionList->m_List.size();
		if (m_warpIndex >= 0) size--;
		int index = rand() % size;
		if (m_warpIndex >= 0 && index >= m_warpIndex) index++;
		m_warpIndex = index;
	}

	// cache wrap pos
	Common::fSendMessageToSetObject(this, m_Data.m_PositionList->m_List.at(m_warpIndex), boost::make_shared<Sonic::Message::MsgGetPosition>(m_warpPos));

	// trail
	m_pGlitterPlayer->PlayOneshot(m_spModel->GetNode("Hips"), "ef_mephiles_warp", 1.0f, 1);
}

void Mephiles::StateWarpAdvance(float dt)
{
	float constexpr c_WarpSpeed = 30.0f;

	hh::math::CVector const oldPos = m_spMatrixNodeTransform->m_Transform.m_Position;
	if (oldPos.isApprox(m_warpPos))
	{
		m_stateNext = ChooseAttackState();
		return;
	}

	hh::math::CVector newPos = oldPos;
	hh::math::CVector direction = m_warpPos - oldPos;
	if (direction.norm() <= c_WarpSpeed * dt)
	{
		newPos = m_warpPos;
	}
	else
	{
		direction.normalize();
		newPos += direction * c_WarpSpeed * dt;
	}

	m_spMatrixNodeTransform->m_Transform.SetPosition(newPos);
	m_spMatrixNodeTransform->NotifyChanged();

	TurnTowardsPlayer(dt);
}

//---------------------------------------------------
// State::Damage
//---------------------------------------------------
void Mephiles::StateDamageBegin()
{
	ChangeState(Shock);

	// TODO: lock on camera?
	if (m_cameraActorID)
	{
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		m_cameraActorID = 0;
	}

	if (m_playDamageVO)
	{
		SubtitleUI::addSubtitle("msg_hint", "hint_bos04_e11_mf");
	}
}

void Mephiles::StateDamageAdvance(float dt)
{
	if (GetCurrentState()->GetStateName() == Wait || (m_enterHalfHP && m_stateTime >= 0.2f) || m_enterLastHP || S06DE_API::GetChaosBoostLevel() == 0)
	{
		m_stateNext = State::Warp;
	}
}

//---------------------------------------------------
// State::HalfHP
//---------------------------------------------------
void Mephiles::StateHalfHPBegin()
{
	m_playedHalfHPVO = false;

	// black fade in
	auto const& spRender = Sonic::CApplicationDocument::GetInstance()->m_pMember->m_spRenderDirectorMTFx;
	if (spRender)
	{
		Hedgehog::Universe::CMessageActor* pRenderObj = (Hedgehog::Universe::CMessageActor*)((uint32_t)spRender.get() + 0x2C);
		SendMessage(pRenderObj->m_ActorID, boost::make_shared<Sonic::Message::MsgStartFadeOut>(0.0f, 0xFF));
		SendMessage(pRenderObj->m_ActorID, boost::make_shared<Sonic::Message::MsgStartFadeIn>(0.1f, 0xFF));
	}

	// teleport to middle
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(hh::math::CQuaternion(0.971765f, 0.0f, 0.23595f, 0.0f), hh::math::CVector(0.0f, 4.0f, 0.0f));
	m_spMatrixNodeTransform->NotifyChanged();

	// teleport player
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	SendMessageImm(context->m_pPlayer->m_ActorID, Sonic::Message::MsgStopActivity());
	SendMessage(context->m_pPlayer->m_ActorID, boost::make_shared<Sonic::Message::MsgSetPosition>(hh::math::CVector(7.2255f, 0.0f, 14.0f)));
	SendMessage(context->m_pPlayer->m_ActorID, boost::make_shared<Sonic::Message::MsgSetRotation>(hh::math::CQuaternion(0.23595f, 0.0f, -0.971765f, 0.0f)));
	Common::SetPlayerVelocity(hh::math::CVector::Zero());

	// shadow avoid Mephiles
	for (auto const& iter : m_shadows)
	{
		SendMessage(iter.first, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(3));
	}
}

void Mephiles::StateHalfHPAdvance(float dt)
{
	switch (m_stateStage)
	{
	case 0:
	{
		// enable camera (after one frame for bone to update transform)
		if (!m_cameraActorID && m_Data.m_CameraPanNoEase && m_stateTime > 0.0f)
		{
			m_cameraActorID = m_Data.m_CameraPanNoEase;
			Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgSetPosition>(hh::math::CVector(2.30794f, 5.2f, 2.46624f)));
			Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
		}

		if (m_stateTime > 0.1f)
		{
			m_stateStage++;
			m_stateTime = 0.0f;

			SubtitleUI::addSubtitle("msg_hint", "hint_bos05_e04_mf");
		}
		break;
	}
	case 1:
	{
		if (m_stateTime >= 0.75f)
		{
			m_stateStage++;
			m_stateTime = 0.0f;

			ChangeState(Command);
		}
		break;
	}
	case 2:
	{
		if (m_stateTime >= 1.0f)
		{
			m_stateStage++;
			m_stateTime = 0.0f;

			ToggleBarrier(true);
		}
		break;
	}
	case 3:
	{
		if (m_stateTime >= 2.0f)
		{
			m_stateStage++;
			m_stateTime = 0.0f;

			ChangeState(Grin);
			FireSphereL();

			// zoom out camera
			if (m_cameraActorID)
			{
				Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
				m_cameraActorID = 0;
			}

			if (m_Data.m_CameraPan)
			{
				m_cameraActorID = m_Data.m_CameraPan;
				Common::fSendMessageToSetObject(this, m_Data.m_CameraPan, boost::make_shared<Sonic::Message::MsgSetPosition>(hh::math::CVector(3.72234f, 3.8f, 3.97765f)));
				Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
			}
		}
		break;
	}
	case 4:
	{
		// pan camera up
		float constexpr cameraMoveUpTime = 2.0f;
		float constexpr cameraMoveUp = 1.0f;
		SetFocusCameraPosition(GetBodyPosition() + hh::math::CVector(0.0f, cameraMoveUp * min(1.0f, m_stateTime / cameraMoveUpTime), 0.0f));

		if (m_stateTime >= 2.5f)
		{
			m_stateStage++;
			m_stateTime = 0.0f;

			ChangeState(Wait);
			m_canDamage = true;

			// resume player
			auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
			SendMessage(context->m_pPlayer->m_ActorID, boost::make_shared<Sonic::Message::MsgReopenActivity>());
			
			// disable shadow avoid Mephiles
			for (auto const& iter : m_shadows)
			{
				SendMessage(iter.first, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(4));
			}

			// throw darksphere
			if (m_darkSphereL)
			{
				SendMessage(m_darkSphereL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
			}

			// swap to lock camera
			if (m_cameraActorID)
			{
				Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
				m_cameraActorID = 0;
			}

			if (m_Data.m_CameraLock)
			{
				m_cameraActorID = m_Data.m_CameraLock;
				Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
			}
		}
		break;
	}
	case 5:
	{
		// wait until darksphere is destroyed
		if (!m_darkSphereL)
		{
			m_stateStage++;
			m_stateTime = 0.0f;

			if (m_cameraActorID)
			{
				Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
				m_cameraActorID = 0;
			}
		}
		break;
	}
	case 6:
	{
		if (m_stateTime >= GetAttackAfterDelay())
		{
			m_stateNext = State::Warp;
		}
		break;
	}
	}

	if (m_stateStage <= 3)
	{
		SetFocusCameraPosition(GetBodyPosition());
	}

	if (m_stateStage >= 3 && !m_playedHalfHPVO && !SubtitleUI::isPlayingSubtitle())
	{
		m_playedHalfHPVO = true;
		SubtitleUI::addSubtitle("msg_hint", "hint_bos04_e10_mf");
	}

	if (m_stateStage >= 5)
	{
		HandleDisableCameraLock();
		TurnTowardsPlayer(dt);
	}

	if (S06DE_API::GetChaosBoostLevel() == 0 && m_stateStage >= 5)
	{
		// only after throwing darksphere
		m_stateNext = State::Dive;
	}
}

void Mephiles::StateHalfHPEnd()
{
	if (m_cameraActorID)
	{
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		m_cameraActorID = 0;
	}

	// destroy darksphere if not already (usually from damage)
	if (m_darkSphereL)
	{
		SendMessage(m_darkSphereL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		m_darkSphereL.reset();
	}
}

//---------------------------------------------------
// State::Dead
//---------------------------------------------------
float const c_mephilesDeadTimeScale = 0.2f;
void Mephiles::StateDeadBegin()
{
	ChangeState(Shock);

	if (m_cameraActorID)
	{
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		m_cameraActorID = 0;
	}

	ScoreManager::addScore(ScoreType::ST_boss);
	SubtitleUI::addSubtitle("msg_hint", "hint_bos04_e11_mf");
	S06DE_API::SetChaosAttackForced(true);

	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	SendMessage(context->m_pPlayer->m_ActorID, boost::make_shared<Sonic::Message::MsgStartOutOfControl>(9999.0f));

	// notify owner is dead
	for (auto& iter : m_shadows)
	{
		SendMessage(iter.first, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(5));
	}
	for (auto& iter : m_shadowsAttached)
	{
		SendMessage(iter.first, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(5));
	}

	ToggleSlowTime(true);

	// set cinematic camera
	hh::math::CVector const playerPos = context->m_spMatrixNode->m_Transform.m_Position;
	float const playerDist = (playerPos - m_spMatrixNodeTransform->m_Transform.m_Position).norm();
	if (playerDist <= 1.5f && m_Data.m_CameraPanNoEase)
	{
		m_cameraActorID = m_Data.m_CameraPanNoEase;

		hh::math::CVector const offset = hh::math::CVector(-3.0f, 1.2f, -1.2f);
		hh::math::CVector const position = playerPos + context->m_spMatrixNode->m_Transform.m_Rotation * offset;
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgSetPosition>(position));
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
		SetFocusCameraPosition(GetBodyPosition());
	}
}

void Mephiles::StateDeadAdvance(float dt)
{
	switch (m_stateStage)
	{
	case 0:
	{
		if (m_stateTime >= c_mephilesDeadTimeScale * 1.5f)
		{
			m_stateTime = 0.0f;
			m_stateStage++;

			// fade out to white
			auto const& spRender = Sonic::CApplicationDocument::GetInstance()->m_pMember->m_spRenderDirectorMTFx;
			if (spRender)
			{
				Hedgehog::Universe::CMessageActor* pRenderObj = (Hedgehog::Universe::CMessageActor*)((uint32_t)spRender.get() + 0x2C);
				SendMessage(pRenderObj->m_ActorID, boost::make_shared<Sonic::Message::MsgStartFadeOut>(c_mephilesDeadTimeScale, 0xFFFFFFFF));
			}
		}
		break;
	}
	case 1:
	{
		if (m_stateTime >= c_mephilesDeadTimeScale)
		{
			m_stateTime = 0.0f;
			m_stateStage++;

			// kill all shadows when screen is white
			for (auto& iter : m_shadows)
			{
				SendMessage(iter.first, boost::make_shared<Sonic::Message::MsgKill>());
			}
			for (auto& iter : m_shadowsAttached)
			{
				SendMessage(iter.first, boost::make_shared<Sonic::Message::MsgKill>());
			}

			m_shadows.clear();
			m_shadowsAttached.clear();

			// teleport player
			auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
			SendMessageImm(context->m_pPlayer->m_ActorID, Sonic::Message::MsgStopActivity());
			SendMessage(context->m_pPlayer->m_ActorID, boost::make_shared<Sonic::Message::MsgSetPosition>(hh::math::CVector(0.0f, m_Data.m_GroundHeight, 0.0f)));
			SendMessage(context->m_pPlayer->m_ActorID, boost::make_shared<Sonic::Message::MsgSetRotation>(hh::math::CQuaternion::Identity()));
			Common::SetPlayerVelocity(hh::math::CVector::Zero());
		}
		break;
	}
	case 2:
	{
		if (m_stateTime >= c_mephilesDeadTimeScale * 0.2f)
		{
			m_stateTime = 0.0f;
			m_stateStage++;

			Common::PlayStageMusic("Dummy", 0.0f);
			Common::PlayEvent(m_pMember->m_pGameDocument->m_pMember->m_spEventManager.get(), "ev705");

			ToggleSlowTime(false);
			LoadingUI::startNowLoading(10.0f);
		}
		break;
	}
	case 3:
	{
		uint32_t pEventManager = (uint32_t)m_pMember->m_pGameDocument->m_pMember->m_spEventManager.get();
		if (!*(bool*)(pEventManager + 288) && !*(bool*)(pEventManager + 292))
		{
			auto const& spMissionManager = m_pMember->m_pGameDocument->m_pGameActParameter->m_spMissionManager;
			if (spMissionManager)
			{
				Hedgehog::Universe::CMessageActor* pMissionManagerObj = (Hedgehog::Universe::CMessageActor*)((uint32_t)spMissionManager.get() + 0x28);
				SendMessage(pMissionManagerObj->m_ActorID, boost::make_shared<Sonic::Message::MsgMissionFinish>());
			}

			LoadingUI::startNowLoading();
			Kill();
		}
		break;
	}
	}
}

//---------------------------------------------------
// State::AttackSphereS
//---------------------------------------------------
void Mephiles::StateAttackSphereSBegin()
{
	if (!m_cameraActorID && m_Data.m_CameraLock)
	{
		m_cameraActorID = m_Data.m_CameraLock;
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	}

	if (GetHPRatio() <= 0.5f)
	{
		m_spawnBarrierTimer = 1.0f;
	}
}

void Mephiles::StateAttackSphereSAdvance(float dt)
{
	float constexpr c_Interval = 1.25f;

	switch (m_stateStage)
	{
	case 0:
	{
		auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
		if ((context->m_Grounded && m_stateTime >= GetAttackBeforeDelay()) || (!context->m_Grounded && m_stateTime >= 1.5f))
		{
			m_stateTime = 0.0f;
			m_stateStage++;
			m_canDamage = true;

			ChangeState(Smile);
			SubtitleUI::addSubtitle("msg_hint", GetHPRatio() <= 0.5f ? "hint_bos04_e05_mf" : "hint_bos04_e09_mf");
		
			FireSphereS();
		}
		break;
	}
	case 1:
	case 2:
	case 3:
	case 4:
	{
		if (m_stateTime >= c_Interval)
		{
			m_stateTime = 0.0f;
			m_stateStage++;

			if (m_stateStage == 5)
			{
				ChangeState(Wait);
			}

			FireSphereS();
		}
		break;
	}
	case 5:
	{
		if (m_cameraActorID && m_stateTime >= GetAttackAfterDelay() - 1.0f)
		{
			Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
			m_cameraActorID = 0;
		}

		if (m_stateTime >= GetAttackAfterDelay())
		{
			m_stateNext = State::Warp;
		}
		break;
	}
	}

	if (m_stateTime >= 0.5f || m_stateStage > 0)
	{
		HandleDisableCameraLock();
	}
	TurnTowardsPlayer(dt);

	if (S06DE_API::GetChaosBoostLevel() == 0)
	{
		m_stateNext = State::Dive;
	}
}

void Mephiles::StateAttackSphereSEnd()
{
	if (m_cameraActorID)
	{
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		m_cameraActorID = 0;
	}
}

void Mephiles::FireSphereS()
{
	SharedPtrTypeless soundHandle;
	Common::ObjectPlaySound(this, 200615007, soundHandle);

	float constexpr c_DarkSphereSpeedS = 20.0f;
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	m_pMember->m_pGameDocument->AddGameObject
	(
		boost::make_shared<DarkSphere>(m_ActorID, context->m_pPlayer->m_ActorID, c_DarkSphereSpeedS, false, GetBodyPosition())
	);
}

//---------------------------------------------------
// State::AttackSphereL
//---------------------------------------------------
void Mephiles::StateAttackSphereLBegin()
{
	if (GetHPRatio() <= 0.5f)
	{
		m_spawnBarrierTimer = 1.0f;
	}
}

void Mephiles::StateAttackSphereLAdvance(float dt)
{
	switch (m_stateStage)
	{
	case 0:
	{
		if (m_stateTime >= GetAttackBeforeDelay())
		{
			m_stateStage++;
			m_stateTime = 0.0f;

			ChangeState(Grin);
			FireSphereL();

			if (!m_cameraActorID && m_Data.m_CameraLock)
			{
				m_cameraActorID = m_Data.m_CameraLock;
				Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
			}

			SubtitleUI::addSubtitle("msg_hint", "hint_bos04_e10_mf");
		}
		break;
	}
	case 1:
	{
		if (m_stateTime >= 2.5f || !m_darkSphereL)
		{
			m_stateStage++;
			m_stateTime = 0.0f;

			ChangeState(Wait);
			m_canDamage = true;

			// throw darksphere
			if (m_darkSphereL)
			{
				SendMessage(m_darkSphereL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
			}
		}
		break;
	}
	case 2:
	{
		// wait until darksphere is destroyed
		if (!m_darkSphereL)
		{
			m_stateStage++;
			m_stateTime = 0.0f;

			if (m_cameraActorID)
			{
				Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
				m_cameraActorID = 0;
			}
		}
		break;
	}
	case 3:
	{
		if (m_stateTime >= GetAttackAfterDelay())
		{
			m_stateNext = State::Warp;
		}
		break;
	}
	}

	if (m_stateTime >= 0.5f || m_stateStage > 0)
	{
		HandleDisableCameraLock();
	}
	TurnTowardsPlayer(dt);

	if (S06DE_API::GetChaosBoostLevel() == 0)
	{
		m_stateNext = State::Dive;
	}
}

void Mephiles::StateAttackSphereLEnd()
{
	if (m_cameraActorID)
	{
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		m_cameraActorID = 0;
	}

	// destroy darksphere if not already (usually from damage)
	if (m_darkSphereL)
	{
		SendMessage(m_darkSphereL->m_ActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		m_darkSphereL.reset();
	}
}

void Mephiles::FireSphereL()
{
	float constexpr c_DarkSphereSpeedL = 5.0f;
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	m_darkSphereL = boost::make_shared<DarkSphere>(m_ActorID, context->m_pPlayer->m_ActorID, c_DarkSphereSpeedL, true, m_spMatrixNodeTransform->m_Transform.m_Position + hh::math::CVector::UnitY() * 3.3f);
	m_pMember->m_pGameDocument->AddGameObject(m_darkSphereL);
}

//---------------------------------------------------
// State::AttackCharge
//---------------------------------------------------
void Mephiles::StateAttackChargeBegin()
{
	if (!m_cameraActorID && m_Data.m_CameraLock)
	{
		m_cameraActorID = m_Data.m_CameraLock;
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
	}

	if (GetHPRatio() <= 0.5f)
	{
		m_spawnBarrierTimer = 1.0f;
	}
}

void Mephiles::StateAttackChargeAdvance(float dt)
{
	switch (m_stateStage)
	{
	case 0:
	{
		if (m_stateTime >= GetAttackBeforeDelay())
		{
			m_stateTime = 0.0f;
			m_stateStage++;

			ChangeState(Command);
			SubtitleUI::addSubtitle("msg_hint", "hint_bos04_e08_mf");
		}
		break;
	}
	case 1:
	{
		if (GetCurrentState()->GetStateName() == Wait)
		{
			SharedPtrTypeless soundHandle;
			Common::ObjectPlaySound(this, 200615000, soundHandle);

			m_stateTime = 0.0f;
			m_stateStage++;
			m_canDamage = true;

			SpawnCircleWait(30, 4.0f, GetAttackBeforeDelay() + 0.5f, 2.0f, MephilesShadow::Type::Charge);
		}
		break;
	}
	case 2:
	{
		if (m_cameraActorID && m_stateTime >= 4.0f)
		{
			Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
			m_cameraActorID = 0;
		}

		if (m_stateTime >= GetAttackAfterDelay() + 4.0f)
		{
			m_stateNext = State::Warp;
		}
		break;
	}
	}

	if (m_stateTime >= 0.6f || m_stateStage > 0)
	{
		HandleDisableCameraLock();
	}
	TurnTowardsPlayer(dt);

	if (S06DE_API::GetChaosBoostLevel() == 0)
	{
		m_stateNext = State::Dive;
	}
}

void Mephiles::StateAttackChargeEnd()
{
	if (m_cameraActorID)
	{
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		m_cameraActorID = 0;
	}
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
	return m_state != State::Hide && m_state != State::Dead && m_HP > 0;
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

void Mephiles::ToggleBarrier(bool enabled)
{
	if (m_barrierID && !enabled)
	{
		m_pGlitterPlayer->StopByID(m_barrierID, false);
		m_barrierID = 0;
	}
	else if (!m_barrierID && enabled)
	{
		SharedPtrTypeless soundHandle;
		Common::ObjectPlaySound(this, 200615023, soundHandle);

		m_barrierID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spModel->GetNode("Hips"), "ef_mephiles_barrier", 1.0f);
	}

	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Barrier", enabled);
}

void Mephiles::PlaySingleVO(std::string const& name, std::string const& id)
{
	if (m_playedVO.count(id) || S06DE_API::GetModelType() != S06DE_API::ModelType::Shadow) return;
	m_playedVO.insert(id);

	SubtitleUI::addSubtitle(name, id);
}

float Mephiles::GetHPRatio() const
{
	return (float)m_HP / (float)m_Data.m_MaxHP;
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

		// smoke effects
		if (m_handLID)
		{
			m_pGlitterPlayer->StopByID(m_handLID, true);
			m_pGlitterPlayer->StopByID(m_handRID, true);
			m_pGlitterPlayer->StopByID(m_footLID, true);
			m_pGlitterPlayer->StopByID(m_footRID, true);
			m_handLID = 0;
			m_handRID = 0;
			m_footLID = 0;
			m_footRID = 0;
		}

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

		// smoke effects
		m_handLID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spModel->GetNode("LeftHand"), "ef_mephiles_smoke", 1.0f);
		m_handRID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spModel->GetNode("RightHand"), "ef_mephiles_smoke", 1.0f);
		m_footLID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spModel->GetNode("LeftFoot"), "ef_mephiles_smoke", 1.0f);
		m_footRID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spModel->GetNode("RightFoot"), "ef_mephiles_smoke", 1.0f);

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
		m_attachCountdown = 0.0f;
		return;
	}

	// follow player's horizontal position
	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector pos = context->m_spMatrixNode->m_Transform.m_Position;
	pos.y() = m_Data.m_GroundHeight;
	m_spMatrixNodeTransform->m_Transform.SetPosition(pos);
	m_spMatrixNodeTransform->NotifyChanged();
}

void Mephiles::TurnTowardsPlayer(float dt)
{
	float constexpr c_TurnRate = 180.0f * DEG_TO_RAD;

	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector newDirection = context->m_spMatrixNode->m_Transform.m_Position - m_spMatrixNodeTransform->m_Transform.m_Position;
	if (newDirection.isApprox(hh::math::CVector::Zero()))
	{
		return;
	}
	
	newDirection.y() = 0.0f; newDirection.normalize();
	hh::math::CVector oldDirection = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ();
	oldDirection.y() = 0.0f; oldDirection.normalize();

	float dot = oldDirection.dot(newDirection);
	Common::ClampFloat(dot, -1.0f, 1.0f);

	float const angle = acos(dot);
	float const maxAngle = dt * c_TurnRate;
	if (angle > maxAngle)
	{
		hh::math::CVector cross = oldDirection.cross(newDirection).normalized();
		Eigen::AngleAxisf rot(maxAngle, cross);
		newDirection = rot * oldDirection;
	}

	hh::math::CQuaternion rotYaw = hh::math::CQuaternion::FromTwoVectors(hh::math::CVector::UnitZ(), newDirection.head<3>());
	m_spMatrixNodeTransform->m_Transform.SetRotation(rotYaw);
	m_spMatrixNodeTransform->NotifyChanged();
}

float Mephiles::GetAttackBeforeDelay() const
{
	float const ratio = GetHPRatio();
	if (ratio <= 1.0f / 3.0f)
	{
		return 0.5f;
	}
	else if (ratio <= 2.0f / 3.0f)
	{
		return 1.0f;
	}
	else
	{
		return 1.5f;
	}
}

float Mephiles::GetAttackAfterDelay() const
{
	float const ratio = GetHPRatio();
	if (ratio <= 1.0f / 3.0f)
	{
		return 1.0f;
	}
	else if (ratio <= 2.0f / 3.0f)
	{
		return 2.0f;
	}
	else
	{
		return 3.0f;
	}
}

Mephiles::State Mephiles::ChooseAttackState()
{
	if (S06DE_API::GetChaosBoostLevel() == 0)
	{
		return State::Dive;
	}
	
	if (m_enterHalfHP)
	{
		m_enterHalfHP = false;
		m_attackCount = 1;
		return State::HalfHP;
	}

	if (m_enterLastHP)
	{
		m_enterLastHP = false;
	}

	m_attackCount++;
	if (m_attackCount % 2 == 0)
	{
		return State::AttackSphereS;
	}
	else if (GetHPRatio() <= 0.5f)
	{
		return State::AttackSphereL;
	}
	else
	{
		return State::AttackCharge;
	}
}

void Mephiles::ToggleSlowTime(bool enable)
{
	auto const spGameplayFlow = (uint32_t)Sonic::CApplicationDocument::GetInstance()->m_pMember->m_pGameplayFlowManager;
	Hedgehog::Universe::CMessageActor* pGameplayFlowMessageActor = (Hedgehog::Universe::CMessageActor*)(spGameplayFlow + 0x60);

	if (enable && !m_slowedTime)
	{
		SendMessage(pGameplayFlowMessageActor->m_ActorID, boost::make_shared<Sonic::Message::MsgChangeGameSpeed>(c_mephilesDeadTimeScale, 0));
		m_slowedTime = true;
	}
	else if (!enable && m_slowedTime)
	{
		SendMessage(pGameplayFlowMessageActor->m_ActorID, boost::make_shared<Sonic::Message::MsgChangeGameSpeed>(1.0f, 1));
		m_slowedTime = false;
	}
}

//---------------------------------------------------
// Camera
//---------------------------------------------------
void Mephiles::SetFocusCameraPosition(hh::math::CVector const& pos)
{
	Common::fSendMessageToSetObject(this, m_Data.m_FocusObject, boost::make_shared<Sonic::Message::MsgSetPosition>(pos));
}

void Mephiles::HandleDisableCameraLock()
{
	if (!m_cameraActorID || (m_cameraActorID != m_Data.m_CameraLock && m_cameraActorID != m_Data.m_CameraLockDive)) return;
	float constexpr minDist = 3.0f;

	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector const playerPos = context->m_spMatrixNode->m_Transform.m_Position;
	hh::math::CVector dir = playerPos - m_spMatrixNodeTransform->m_Transform.m_Position;
	dir.y() = 0.0f;
	if (dir.norm() < minDist)
	{
		Common::fSendMessageToSetObject(this, m_cameraActorID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(7));
		m_cameraActorID = 0;
	}
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
	m_spawnedActors.clear();
	m_spawnType = MephilesShadow::Type::Encirclement;
}

void Mephiles::SpawnCircleWait(int count, float radius, float attackStartTime, float attackMaxDelay, MephilesShadow::Type attackType)
{
	m_maxSpawnCount = count;
	m_spawnCount = 0;
	m_spawnTimer = 0.0f;
	m_spawnRadius = radius;
	m_spawnedActors.clear();
	m_attackStartTime = attackStartTime;
	m_attackMaxDelay = attackMaxDelay;
	m_spawnType = attackType;
}

void Mephiles::AdvanceShadowSpawn(float dt)
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
				// prevent deleting actors just spawned
				auto iter = m_shadows.begin();
				do
				{
					iter = m_shadows.begin();
					std::advance(iter, rand() % m_shadows.size());
				} while (m_spawnedActors.count(iter->first));

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
			case MephilesShadow::Type::Charge:
			{
				angle = 0.01f + 2.0f * PI_F * (float)m_spawnCount / (float)m_maxSpawnCount;
				break;
			}
			}

			auto spShadow = boost::make_shared<MephilesShadow>(m_ActorID, m_spawnType, m_spawnRadius, angle, m_Data.m_GroundHeight, m_spMatrixNodeTransform->m_Transform.m_Position);
			m_pMember->m_pGameDocument->AddGameObject(spShadow);
			m_shadows[spShadow->m_ActorID] = spShadow;
			m_spawnedActors.insert(spShadow->m_ActorID);

			// any spawn other than Encirclement will circle around Mephiles
			if (m_spawnType != MephilesShadow::Type::Encirclement)
			{
				spShadow->SetInitialStateSpring(m_attackStartTime, m_attackMaxDelay);
			}
			
			m_spawnCount++;
			m_spawnTimer -= spawnRate;
		}
	}
}

void Mephiles::AdvanceShadowExplode(float dt)
{
	if (m_attachCountdown > 0.0f && !m_shadowsAttached.empty())
	{
		m_attachCountdown -= dt;
		if (m_attachCountdown <= 0.0f)
		{
			// play explosion sfx on player
			SharedPtrTypeless handle;
			Common::SonicContextPlaySound(handle, 200614014, 0);
			m_attachSfx.reset();

			// damage player
			auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
			SendMessage(context->m_pPlayer->m_ActorID, boost::make_shared<Sonic::Message::MsgDamage>
				(
					*(uint32_t*)0x1E0BE28, // DamageID_SonicHeavy
					context->m_spMatrixNode->m_Transform.m_Position,
					hh::math::CVector::Zero()
				)
			);

			// kill shadows without effects
			for (auto& iter : m_shadowsAttached)
			{
				SendMessage(iter.first, boost::make_shared<Sonic::Message::MsgKill>());
			}
			m_shadowsAttached.clear();

			// pfx
			auto attackEffectNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
			attackEffectNode->SetParent(Sonic::CApplicationDocument::GetInstance()->m_pMember->m_spMatrixNodeRoot.get());
			attackEffectNode->m_Transform.SetPosition(context->m_spMatrixNode->m_Transform.m_Position + hh::math::CVector::UnitY() * 0.5f);
			attackEffectNode->NotifyChanged();
			m_pGlitterPlayer->PlayOneshot(attackEffectNode, "ef_mephiles_spherebomb_s", 1.0f, 1);
		}
	}
	else if (m_attachSfx)
	{
		m_attachSfx.reset();
	}
}
