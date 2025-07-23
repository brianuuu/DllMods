#include "EnemyShield.h"

EnemyShield::EnemyShield
(
	hh::mr::CTransform const& startTrans, 
	bool isMonster // = false
)
	: m_isMonster(isMonster)
{
	// initial transform
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(startTrans.m_Rotation, startTrans.m_Position);
	m_spMatrixNodeTransform->NotifyChanged();
}

bool EnemyShield::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	char const* name = "en_shield";

	// model
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(name, 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, true);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, name);
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo("Default", name, 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModel->BindPose(m_spAnimPose);

	// states
	SetContext(this);
	AddAnimationState("Default");
	ChangeState("Default");

	// uv-anim
	m_spEffectMotionAll = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
	m_spModel->BindEffect(m_spEffectMotionAll);

	FUNCTION_PTR(void, __thiscall, fpGetTexCoordAnimData, 0x7597E0,
		hh::mot::CMotionDatabaseWrapper const& wrapper,
		boost::shared_ptr<Hedgehog::Motion::CTexcoordAnimationData>&texCoordAnimData,
		hh::base::CSharedString const& name,
		uint32_t flag
	);

	FUNCTION_PTR(void, __thiscall, fpCreateUVAnim, 0x7537E0,
		Hedgehog::Motion::CSingleElementEffectMotionAll * This,
		boost::shared_ptr<hh::mr::CModelData> const& modelData,
		boost::shared_ptr<Hedgehog::Motion::CTexcoordAnimationData> const& texCoordAnimData
	);

	hh::mot::CMotionDatabaseWrapper motWrapper(in_spDatabase.get());
	boost::shared_ptr<Hedgehog::Motion::CTexcoordAnimationData> texCoordAnimData;
	fpGetTexCoordAnimData(motWrapper, texCoordAnimData, name, 0);
	fpCreateUVAnim(m_spEffectMotionAll.get(), spModelBaseData, texCoordAnimData);

	return true;
}

void EnemyShield::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	SharedPtrTypeless soundHandle;
	Common::ObjectPlaySound(this, 200614002, soundHandle);
}

bool EnemyShield::ProcessMessage
(
	Hedgehog::Universe::Message& message, bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgRestartStage>() || message.Is<Sonic::Message::MsgStageClear>())
		{
			Kill();
			return true;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void EnemyShield::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	// alpha
	float constexpr alphaStart = 20.0f / 60.0f;
	float constexpr alphaEnd = 0.5f;
	if (m_lifetime > alphaStart)
	{
		m_spEffectMotionAll->m_ForceAlphaColor.w() = max(0.0f, (alphaEnd - m_lifetime) / (alphaEnd - alphaStart));
		m_spEffectMotionAll->m_EnableForceAlphaColor = true;
	}

	// uv-anim
	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
	fpUpdateMotionAll(m_spEffectMotionAll.get(), in_rUpdateInfo.DeltaTime);

	// animation
	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);

	// timeout
	m_lifetime += in_rUpdateInfo.DeltaTime;
	if (m_lifetime > alphaEnd + 0.1f)
	{
		Kill();
	}
}
