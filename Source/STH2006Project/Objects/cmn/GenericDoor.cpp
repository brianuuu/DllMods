#include "GenericDoor.h"

BB_SET_OBJECT_MAKE_HOOK(GenericDoor)
void GenericDoor::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(GenericDoor);
}

GenericDoor::~GenericDoor()
{
	if (m_Data.m_Name)
	{
		m_Data.m_Name->Release();
	}
}

void GenericDoor::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	uint32_t dummy = 0u;
	char const* name = "Name";
	m_Data.m_Name = Sonic::CParamTypeList::Create(&dummy, name);
	m_Data.m_Name->m_pMember->m_DefaultValue = 1;
	m_Data.m_Name->m_pMember->m_pFuncData->m_ValueMax = 1;
	if (m_Data.m_Name)
	{
		m_Data.m_Name->AddRef();
	}
	in_rEditParam.CreateParamBase(m_Data.m_Name, name);

	in_rEditParam.CreateParamFloat(&m_Data.m_CullingRange, "CullingRange");
	in_rEditParam.CreateParamInt(&m_Data.m_CueID, "CueID");
	in_rEditParam.CreateParamBool(&m_Data.m_DefaultTriggered, "DefaultTriggered");
}

bool GenericDoor::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(m_Data.m_Name->m_pMember->m_DefaultValueName, 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_pMember->m_CastShadow);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, m_Data.m_Name->m_pMember->m_DefaultValueName);
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo("Trigger", m_Data.m_Name->m_pMember->m_DefaultValueName.c_str(), 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModel->BindPose(m_spAnimPose);

	// mat-anim
	m_spEffectMotionAll = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
	m_spModel->BindEffect(m_spEffectMotionAll);
	if (m_Data.m_Name->m_pMember->m_DefaultValueName == "wvo_obj_doorA")
	{
		AddMatAnim("wvo_obj_doorA_light_close");
	}

	SetCullingRange(m_Data.m_CullingRange);

	// states
	SetContext(this);
	AddAnimationState("Trigger");
	if (m_triggered)
	{
		Trigger(true);
	}

	return true;
}

bool GenericDoor::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body, must have Reference bone, ColID_Basic
	auto const attachNode = m_spModel->GetNode("Collision");
	AddRigidBody(m_spRigidBody, m_Data.m_Name->m_pMember->m_DefaultValueName.c_str(), m_Data.m_Name->m_pMember->m_DefaultValueName.c_str(), *(int*)0x1E0AFF0, attachNode, in_spDatabase);

	return true;
}

void GenericDoor::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder,
	Sonic::CGameDocument* in_pGameDocument,
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	if (!m_useBank)
	{
		m_triggered = m_Data.m_DefaultTriggered;
		m_useBank = true;
	}

	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);
}

void GenericDoor::AddParameterBank
(
	const Hedgehog::Base::CRefPtr<Sonic::CParameterBank>& in_rParameterBank
)
{
	in_rParameterBank->AccessParameterBankBool("Triggered", &m_triggered);
	in_rParameterBank->AccessParameterBankBool("UseBank", &m_useBank);
}

void GenericDoor::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
	if (m_effectEndTime > 0.0f)
	{
		m_effectTime += in_rUpdateInfo.DeltaTime;
	}
	if (m_effectTime <= m_effectEndTime)
	{
		fpUpdateMotionAll(m_spEffectMotionAll.get(), in_rUpdateInfo.DeltaTime);
	}

	Update(in_rUpdateInfo);
	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);

	if (m_sfx && Common::IsAnimationFinished(this))
	{
		m_sfx.reset();
	}
}

bool GenericDoor::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (!flag)
	{
		return Sonic::CObjectBase::ProcessMessage(message, flag);
	}

	if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
	{
		auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
		if (!m_triggered && msg.m_Event == 6)
		{
			Trigger();
		}
		return true;
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void GenericDoor::Trigger(bool skip)
{
	m_triggered = true;
	ChangeState("Trigger");

	// special cases
	if (m_Data.m_Name->m_pMember->m_DefaultValueName == "wvo_obj_doorA")
	{
		m_effectEndTime = 1.0f;
		m_spEffectMotionAll->m_MaterialMotionList.clear();
		AddMatAnim("wvo_obj_doorA_light_open", skip);
	}

	// jump to animation end
	if (skip)
	{
		m_effectTime = m_effectEndTime;
		m_spAnimPose->Update(1000.0f);
		return;
	}

	// play sfx if provided
	if (m_Data.m_CueID)
	{
		Common::ObjectPlaySound(this, m_Data.m_CueID, m_sfx);
	}
}

void GenericDoor::AddMatAnim(std::string const& name, bool skip)
{
	FUNCTION_PTR(void, __thiscall, fpGetMaterialAnimData, 0x759720,
		hh::mot::CMotionDatabaseWrapper const& wrapper,
		boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData>&materialAnimData,
		hh::base::CSharedString const& name,
		uint32_t flag
	);

	FUNCTION_PTR(void, __thiscall, fpCreateMatAnim, 0x753910,
		Hedgehog::Motion::CSingleElementEffectMotionAll * This,
		boost::shared_ptr<hh::mr::CModelData> const& modelData,
		boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData> const& materialAnimData
	);

	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);

	hh::mot::CMotionDatabaseWrapper motWrapper(m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());
	boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData> materialAnimData;
	fpGetMaterialAnimData(motWrapper, materialAnimData, name.c_str(), 0);
	fpCreateMatAnim(m_spEffectMotionAll.get(), m_spModel->m_spModel, materialAnimData);
	fpUpdateMotionAll(m_spEffectMotionAll.get(), skip ? m_effectEndTime : 0.0f);
}
