#include "GadgetGunSimple.h"
#include "GadgetMissile.h"

bool GadgetGunSimple::SetAddRenderables
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
	ChangeState("Fire");
	m_spAnimPose->Update(1.0f);

	// set initial transform
	UpdateTransform();

	SetCullingRange(0.0f);

	return true;
}

bool GadgetGunSimple::ProcessMessage
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
				m_loadTimer = 0.0f;
				ChangeState("Load");
				break;
			}
			case 7:
			{
				m_loadTimer = -1.0f;
				if (GetCurrentState()->GetStateName() != "Fire")
				{
					ChangeState("Fire");
				}
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

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void GadgetGunSimple::UpdateParallel
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

bool GadgetGunSimple::IsLoaded() const
{
	return m_loadTimer <= 0.0f && GetCurrentState()->GetStateName() == "Load" && Common::IsAnimationFinished(this);
}

bool GadgetGunSimple::IsStarted() const
{
	return m_started && m_loadTimer >= 0.0f;
}

void GadgetGunSimple::FireMissile()
{
	if (m_loadTimer > 0.0f) return;
	m_loadTimer = m_reloadTime;

	SharedPtrTypeless sfx;
	Common::ObjectPlaySound(this, 200612006, sfx);

	auto node = m_spModel->GetNode("MissilePoint");
	hh::mr::CTransform startTrans;
	startTrans.m_Rotation = hh::math::CQuaternion(node->GetWorldMatrix().rotation());
	startTrans.m_Position = node->GetWorldMatrix().translation();
	m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<GadgetMissile>(m_owner, startTrans));
}

void GadgetGunSimple::UpdateTransform()
{
	// follow attach point so sound can work
	hh::math::CMatrix const matrix = m_spNodeParent->GetWorldMatrix();
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(hh::math::CQuaternion(matrix.rotation()), matrix.translation());
	m_spMatrixNodeTransform->NotifyChanged();
}
