#include "GadgetGun.h"

#include "Objects/enemy/EnemyBullet.h"

bool GadgetGun::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument,
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// model
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(m_name.c_str(), 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spNodeParent);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_castShadow);

	// animations
	std::string const rev = m_name + "_rev";
	std::string const fire = m_name + "_Fire";
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, m_name.c_str());
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo("Load", m_name.c_str(), 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo("Unload", rev.c_str(), 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	entries.push_back(hh::anim::SMotionInfo("Fire", fire.c_str(), 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModel->BindPose(m_spAnimPose);

	// states
	SetContext(this);
	AddAnimationState("Load");
	AddAnimationState("Unload");
	AddAnimationState("Fire");
	ChangeState("Unload");
	m_spAnimPose->Update(0.35f); // 21 frames

	// set initial transform
	UpdateTransform();

	SetCullingRange(0.0f);

	return true;
}

bool GadgetGun::ProcessMessage
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
			case 0:
			{
				m_loaded = false;
				break;
			}
			case 1:
			{
				m_loaded = true;
				break;
			}
			case 6:
			{
				m_started = true;
				break;
			}
			case 7:
			{
				m_started = false;
				break;
			}
			}
			return true;
		}
	}

	return Sonic::CGameObject3D::ProcessMessage(message, flag);
}

void GadgetGun::UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo)
{
	// handle animations
	hh::base::CSharedString const currentState = GetCurrentState()->GetStateName();
	bool const animationFinished = Common::IsAnimationFinished(this);

	if (currentState == "Unload" && animationFinished)
	{
		if (!m_sfxPlayed)
		{
			m_sfxPlayed = true;
			auto const attachNodeL = m_spModel->GetNode(m_isRight ? "pReload_R" : "pReload_L");
			m_pGlitterPlayer->PlayOneshot(attachNodeL, "ef_hover_reload", 1.0f, 1);
		}

		if (m_loaded && m_started)
		{
			ChangeState("Load");
		}
	}
	else if (currentState != "Unload" && (!m_loaded || !m_started))
	{
		ChangeState("Unload");
		m_sfxPlayed = false;
	}

	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);
	UpdateTransform();
}

bool GadgetGun::IsReady() const
{
	hh::base::CSharedString const currentState = GetCurrentState()->GetStateName();
	return (currentState == "Load" && Common::IsAnimationFinished(this)) || currentState == "Fire";
}

bool GadgetGun::CanUnload() const
{
	hh::base::CSharedString const currentState = GetCurrentState()->GetStateName();
	return currentState != "Unload" && Common::IsAnimationFinished(this);
}

void GadgetGun::UpdateTransform()
{
	// follow attach point so sound can work
	hh::math::CMatrix const matrix = m_spNodeParent->GetWorldMatrix();
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(hh::math::CQuaternion(matrix.rotation()), matrix.translation());
	m_spMatrixNodeTransform->NotifyChanged();
}

void GadgetGun::FireBullet()
{
	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612014, sfx);

	if (Common::IsAnimationFinished(this))
	{
		ChangeState("Fire");
	}

	auto node = m_spModel->GetNode(m_isRight ? "MissilePoint_R" : "MissilePoint_L");
	hh::mr::CTransform startTrans;
	startTrans.m_Rotation = m_spMatrixNodeTransform->m_Transform.m_Rotation;
	startTrans.m_Position = node->GetWorldMatrix().translation();
	m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<EnemyBullet>(m_owner, startTrans));
}
