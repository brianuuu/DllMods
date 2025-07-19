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
}

void Mephiles::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	switch (m_state)
	{
	case State::Appear: StateAppearAdvance(in_rUpdateInfo.DeltaTime); break;
	case State::Hide: StateHideAdvance(in_rUpdateInfo.DeltaTime); break;
	}

	m_stateTime += in_rUpdateInfo.DeltaTime;
	if (m_stateNext != m_state)
	{
		HandleStateChange();
	}

	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);
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
			// put player at fixed distance from Mephiles
			hh::math::CVector otherPos = m_spMatrixNodeTransform->m_Transform.m_Position;
			SendMessageImm(message.m_SenderActorID, Sonic::Message::MsgGetPosition(otherPos));
			otherPos = m_spMatrixNodeTransform->m_Transform.m_Position + (otherPos - m_spMatrixNodeTransform->m_Transform.m_Position).normalized() * (S06DE_API::GetChaosBoostLevel() > 0 ? 2.0f : 1.5f);
			SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamageSuccess>(otherPos, true));

			if (CanDamage())
			{
				// TODO:
			}
			else
			{
				CreateShield(message.m_SenderActorID);
			}
			return true;
		}
		
		if (message.Is<Sonic::Message::MsgNotifyShockWave>())
		{
			if (CanDamage())
			{
				// TODO:
			}
			else
			{
				CreateShield(message.m_SenderActorID);
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
						m_spMatrixNodeTransform->m_Transform.m_Position,
						hh::math::CVector::Zero()
					)
				);
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
			*msg.m_pPosition = m_spModel->GetNode("Hips")->GetWorldMatrix().translation();
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
			m_shadowManager.SpawnEncirclement(200, 15.0f);
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
			if (m_shadowManager.m_numKilledUnit >= 90)
			{
				PlaySingleVO("msg_hint", "hint_bos04_e03_mf");
			}
			else if (m_shadowManager.m_numKilledUnit >= 60)
			{
				PlaySingleVO("msg_hint", "hint_bos04_a00_sd");
			}
			else if (m_shadowManager.m_numKilledUnit >= 30)
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
			m_stateTime = 0.0f;
			if (m_shadowManager.m_numUnit < 100 || m_stateStage == 3)
			{
				// too few shadows
				m_shadowManager.SpawnEncirclement(50, (m_stateStage == 3) ? 25.0f : 20.0f);
				m_stateStage++;
			}
			else
			{
				float constexpr MinAppearRadius = 5.0f;
				float constexpr MaxAppearRadius = 7.5f;
				float const radius = RAND_FLOAT(MinAppearRadius, MaxAppearRadius);
				if (m_shadowManager.m_numKilledUnit >= 90)
				{
					m_shadowManager.SpawnSpring(50, radius, 1.0f, 2.0f);
				}
				else if (m_shadowManager.m_numKilledUnit >= 60)
				{
					m_shadowManager.SpawnSpring(40, radius, 1.5f, 1.5f);
				}
				else if (m_shadowManager.m_numKilledUnit >= 30)
				{
					m_shadowManager.SpawnSpring(30, radius, 2.0f, 1.0f);
				}
				else
				{
					m_shadowManager.SpawnSpring(20, radius, 2.5f, 0.5f);
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
bool Mephiles::CanLock() const
{
	return m_state != State::Hide;
}

bool Mephiles::CanDamage() const
{
	return false;
}

void Mephiles::CreateShield(uint32_t otherActor) const
{
	hh::mr::CTransform startTrans{};
	if (!otherActor || !SendMessageImm(otherActor, Sonic::Message::MsgGetPosition(startTrans.m_Position)))
	{
		return;
	}

	bool const isPlayer = SendMessageImm(otherActor, Sonic::Message::MsgGetPlayerType());
	hh::math::CVector const bodyCenter = m_spModel->GetNode("Hips")->GetWorldMatrix().translation();

	hh::math::CVector dir = hh::math::CVector::UnitZ();
	if (isPlayer)
	{
		// player compare with root position
		dir = (startTrans.m_Position - m_spMatrixNodeTransform->m_Transform.m_Position).normalized();
	}
	else
	{
		// normal objects compare with body center
		dir = (startTrans.m_Position - bodyCenter).normalized();
	}
	
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

	startTrans.m_Position = bodyCenter + dir * 0.75f;
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
// Shadow Manager
//---------------------------------------------------
void Mephiles::ShadowManager::SpawnEncirclement(int count, float radius)
{

}

void Mephiles::ShadowManager::SpawnSpring(int count, float radius, float attackStartTime, float maxDelay)
{
}

void Mephiles::ShadowManager::Advance(float dt)
{

}
