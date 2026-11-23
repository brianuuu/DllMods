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
		m_Data.m_Name->AddRef();
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

	// states
	SetContext(this);
	AddAnimationState("Trigger");

	// mat-anim
	m_spEffectMotionAll = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
	m_spModel->BindEffect(m_spEffectMotionAll);

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

	hh::mot::CMotionDatabaseWrapper motWrapper(in_spDatabase.get());
	boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData> materialAnimData;
	if (m_Data.m_Name->m_pMember->m_DefaultValueName == "wvo_obj_doorA")
	{
		fpGetMaterialAnimData(motWrapper, materialAnimData, "wvo_obj_doorA_light", 0);
		fpCreateMatAnim(m_spEffectMotionAll.get(), spModelBaseData, materialAnimData);
	}

	SetCullingRange(m_Data.m_CullingRange);

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

void GenericDoor::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
	fpUpdateMotionAll(m_spEffectMotionAll.get(), in_rUpdateInfo.DeltaTime);

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
		if (msg.m_Event == 6)
		{
			ChangeState("Trigger");
			if (m_Data.m_CueID)
			{
				Common::ObjectPlaySound(this, m_Data.m_CueID, m_sfx);
			}
		}
		return true;
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}
