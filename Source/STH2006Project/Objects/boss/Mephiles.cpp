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
	entries.push_back(hh::anim::SMotionInfo("Loop", "en_shwait_fmef_Root", 1.0f, hh::anim::eMotionRepeatType_Loop));
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
	hk2010_2_0::hkpSphereShape* damageShape = new hk2010_2_0::hkpSphereShape(0.9f);
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
}

void Mephiles::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	switch (m_state)
	{
	case State::Hide: StateHideAdvance(in_rUpdateInfo.DeltaTime); break;
	}

	if (m_stateNext != m_state)
	{
		HandleStateChange();
	}

	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);

	// initial VO
	if (!m_playedInitVO && !LoadingUI::IsEnabled())
	{
		m_playedInitVO = true;
		if (m_encounterCount < 3 && S06DE_API::GetModelType() == S06DE_API::ModelType::Shadow)
		{
			mst::TextEntry entry{};
			switch (m_encounterCount)
			{
			case 0: entry = MstManager::GetSubtitle("msg_hint", "hint_bos04_e00_sd"); break;
			case 1: entry = MstManager::GetSubtitle("msg_hint_xenon", "hint_bos04_a08_sd"); break;
			case 2: entry = MstManager::GetSubtitle("msg_hint", "hint_bos04_a02_sd"); break;
			}
			SubtitleUI::addSubtitle(entry);
		}
		m_encounterCount++;
	}
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
	case State::Init: StateInitEnd(); break;
	case State::Hide: StateHideEnd(); break;
	}

	// start next state
	switch (m_stateNext)
	{
	case State::Hide: StateHideBegin(); break;
	}

	m_state = m_stateNext;
}

//---------------------------------------------------
// State::Init
//---------------------------------------------------
void Mephiles::StateInitEnd()
{
	// Remove shadow from player
	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	auto& bundle = m_pMember->m_pWorld->m_pMember->m_BundlePairMap;
	bundle.at("SMO")->m_pBundle->RemoveRenderable(context->m_pPlayer->m_spCharacterModel);
}

//---------------------------------------------------
// State::Hide
//---------------------------------------------------
void Mephiles::StateHideBegin()
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
}

void Mephiles::StateHideAdvance(float dt)
{
	if (S06DE_API::GetChaosBoostLevel() > 0)
	{
		mst::TextEntry const entry = MstManager::GetSubtitle("msg_hint", m_hasEjected ? "hint_bos04_e12_mf" : "hint_bos04_e06_mf");
		SubtitleUI::addSubtitle(entry);

		m_stateNext = State::Ejected;
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

void Mephiles::StateHideEnd()
{
	// add model rendering back
	Sonic::CGameObject::AddRenderable("Object", m_spModel, false);
	m_spAnimPose->m_Scale = 1.0f;

	// re-enable collision
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Body", true);
	Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Damage", true);
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
	hh::math::CVector const bodyCenter = m_spMatrixNodeTransform->m_Transform.m_Position + hh::math::CVector::UnitY() * 0.5f;

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
