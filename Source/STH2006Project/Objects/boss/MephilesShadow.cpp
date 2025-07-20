#include "MephilesShadow.h"

MephilesShadow::MephilesShadow
(
	uint32_t owner, 
	Type type,
	hh::math::CVector const& startPos
)
	: m_owner(owner)
	, m_type(type)
{
	m_spMatrixNodeTransform->m_Transform.SetPosition(startPos);
	FacePlayer();
}

char const* MephilesShadow::Loop = "Loop";

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
	hk2010_2_0::hkpSphereShape* rigidBodyShape = new hk2010_2_0::hkpSphereShape(0.8f);
	AddEventCollision("Body", rigidBodyShape, *(int*)0x1E0AF24, true, m_spNodeBody); // CollisionTypeEnemyAndLockOn

	// damage body
	hk2010_2_0::hkpSphereShape* damageShape = new hk2010_2_0::hkpSphereShape(0.4f);
	AddEventCollision("Damage", damageShape, *(int*)0x1E0AFD8, true, m_spNodeBody); // ColID_PlayerEvent

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
			TakeDamage(message.m_SenderActorID);
			SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamageSuccess>(GetBodyPosition(), true));
			return true;
		}

		if (message.Is<Sonic::Message::MsgNotifyShockWave>())
		{
			// TODO:
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
			return true;
		}

		if (message.Is<Sonic::Message::MsgGetEnemyType>())
		{
			auto& msg = static_cast<Sonic::Message::MsgGetEnemyType&>(message);
			*msg.m_pType = 1;
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
	}

	m_stateTime += in_rUpdateInfo.DeltaTime;
	if (m_stateNext != m_state)
	{
		HandleStateChange();
	}

	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);
}

void MephilesShadow::HandleStateChange()
{
	// end current state
	switch (m_state)
	{
	case State::Idle: break;
	}

	// start next state
	switch (m_stateNext)
	{
	}

	m_state = m_stateNext;
	m_stateTime = 0.0f;
}

//---------------------------------------------------
// State::Idle
//---------------------------------------------------
void MephilesShadow::StateIdleAdvance(float dt)
{
	FacePlayer();
}

//---------------------------------------------------
// State::Blown
//---------------------------------------------------
void MephilesShadow::StateBlownBegin(float dt)
{
}

void MephilesShadow::StateBlownAdvance(float dt)
{
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

void MephilesShadow::FacePlayer()
{
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector const playerPos = context->m_spMatrixNode->m_Transform.m_Position;
	hh::math::CVector dir = playerPos - m_spMatrixNodeTransform->m_Transform.m_Position;
	dir.y() = 0.0f;
	dir.normalize();

	hh::math::CQuaternion const rotation = hh::math::CQuaternion::FromTwoVectors(hh::math::CVector::UnitZ(), dir.head<3>());
	m_spMatrixNodeTransform->m_Transform.SetRotation(rotation);
	m_spMatrixNodeTransform->NotifyChanged();
}

void MephilesShadow::TakeDamage(uint32_t otherActor)
{
	hh::math::CVector otherPos = hh::math::CVector::Zero();
	SendMessageImm(otherActor, Sonic::Message::MsgGetPosition(otherPos));

	// TODO: velocity

	// TODO: notify owner

	m_stateNext = State::Blown;
}
