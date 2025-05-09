#include "Hint.h"

#include "MstManager.h"
#include "SubtitleUI.h"

BB_SET_OBJECT_MAKE_HOOK(Hint)
void Hint::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(Hint);
}

Hint::~Hint()
{
	if (m_Data.m_HintName)
	{
		m_Data.m_HintName->Release();
	}

	if (m_Data.m_HintTimes)
	{
		m_Data.m_HintTimes->Release();
	}
}

void Hint::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	uint32_t dummy = 0u;
	char const* hintName = "HintName";
	m_Data.m_HintName = Sonic::CParamTypeList::Create(&dummy, hintName);
	m_Data.m_HintName->m_pMember->m_DefaultValue = 1;
	m_Data.m_HintName->m_pMember->m_pFuncData->m_ValueMax = 1;
	if (m_Data.m_HintName)
	{
		m_Data.m_HintName->AddRef();
	}
	in_rEditParam.CreateParamBase(m_Data.m_HintName, hintName);

	char const* hintTimes = "HintTimes";
	m_Data.m_HintTimes = Sonic::CParamTypeList::Create(&dummy, hintTimes);
	m_Data.m_HintTimes->m_pMember->m_DefaultValue = 1;
	m_Data.m_HintTimes->m_pMember->m_pFuncData->m_ValueMax = 1;
	if (m_Data.m_HintTimes)
	{
		m_Data.m_HintTimes->AddRef();
	}
	in_rEditParam.CreateParamBase(m_Data.m_HintTimes, hintTimes);

	in_rEditParam.CreateParamInt(&m_Data.m_Type, "Type");
	in_rEditParam.CreateParamFloat(&m_Data.m_CollisionSize.x(), "Collision_Width");
	in_rEditParam.CreateParamFloat(&m_Data.m_CollisionSize.y(), "Collision_Height");
	in_rEditParam.CreateParamFloat(&m_Data.m_CollisionSize.z(), "Collision_Length");
	in_rEditParam.CreateParamBool(&m_Data.m_UseVoiceTime, "UseVoiceTime");
	in_rEditParam.CreateParamBase(Sonic::CParamTarget::Create(&m_Data.m_NextHintID), "NextHintID");
}

bool Hint::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	if (m_Data.m_Type != (int)Type::Default)
	{
		// no model
		return true;
	}

	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base model
	char const* modelName = "cmn_hint";
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(modelName, 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_pMember->m_CastShadow);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, modelName);
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo("Wait", modelName, 1.0f, hh::anim::eMotionRepeatType_Loop));
	entries.push_back(hh::anim::SMotionInfo("Play", "cmn_hint_re", 1.0f, hh::anim::eMotionRepeatType_Loop));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModel->BindPose(m_spAnimPose);

	// states
	SetContext(this);
	AddAnimationState("Wait");
	AddAnimationState("Play");
	SetAnimationBlend("Wait", "Play", 0.5f);
	SetAnimationBlend("Play", "Wait", 0.5f);
	ChangeState("Wait");

	return true;
}

bool Hint::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	if (m_Data.m_Type == (int)Type::EventOnly)
	{
		return true;
	}

	// event collision
	hk2010_2_0::hkpBoxShape* shapeEventTrigger = new hk2010_2_0::hkpBoxShape(m_Data.m_CollisionSize * 0.5f);
	AddEventCollision("Collision", shapeEventTrigger, *(int*)0x1E0AFD8, true, m_spMatrixNodeTransform);

	return true;
}

void Hint::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder,
	Sonic::CGameDocument* in_pGameDocument,
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	// cache .mst file and get subtitle entry
	MstManager::RequestMst("msg_hint");
	m_entry = MstManager::GetSubtitle("msg_hint", m_Data.m_HintName->m_pMember->m_DefaultValueName.c_str());
}

void Hint::GetObjectTriggerType
(
	Hedgehog::vector<uint32_t>& in_rTriggerTypeList
)
{
	FUNCTION_PTR(int, __stdcall, fpSetUpTrigger4, 0xEA2940, Hedgehog::vector<uint32_t>&in_rTriggerTypeList);
	fpSetUpTrigger4(in_rTriggerTypeList);
}

void Hint::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	if (m_isPlaying)
	{
		m_timer -= in_rUpdateInfo.DeltaTime;
		if (m_timer <= 0.0f)
		{
			m_isPlaying = false;
			Common::fEventTrigger(this, 4);

			// trigger next hint
			if (m_Data.m_NextHintID)
			{
				Common::fSendMessageToSetObject(this, m_Data.m_NextHintID, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(6));
			}

			if (m_Data.m_Type == (int)Type::Default)
			{
				ChangeState("Wait");
			}
			else
			{
				// invisible ones kill itself
				Common::fDestroyGameObject(this);
				return;
			}
		}
	}

	if (m_Data.m_Type == (int)Type::Default)
	{
		m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
		Update(in_rUpdateInfo);
	}
}

bool Hint::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (!flag)
	{
		return Sonic::CObjectBase::ProcessMessage(message, flag);
	}

	if (message.Is<Sonic::Message::MsgHitEventCollision>())
	{
		PlayHint();
		return true;
	}

	if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
	{
		auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
		if (msg.m_Event == 6)
		{
			PlayHint();
		}
		return true;
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void Hint::PlayHint()
{
	if (m_isPlaying) return;

	// split time string into floats
	std::vector<float> times;
	if (!m_Data.m_UseVoiceTime && !m_Data.m_HintTimes->m_pMember->m_DefaultValueName.empty())
	{
		size_t pos = 0;
		std::string timesStr = m_Data.m_HintTimes->m_pMember->m_DefaultValueName.c_str();
		while ((pos = timesStr.find(',')) != std::string::npos)
		{
			times.push_back(std::stof(timesStr.substr(0, pos)));
			timesStr.erase(0, pos + 1);
		}
	}

	m_isPlaying = true;
	m_timer = SubtitleUI::addSubtitle(m_entry, times);

	if (m_Data.m_Type == (int)Type::Default)
	{
		ChangeState("Play");

		SharedPtrTypeless sfx;
		Common::ObjectPlaySound(this, 200600002, sfx);
	}
}
