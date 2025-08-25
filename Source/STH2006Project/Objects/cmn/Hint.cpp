#include "Hint.h"

#include "Managers/MstManager.h"
#include "UI/SubtitleUI.h"

BB_SET_OBJECT_MAKE_HOOK(Hint)
void Hint::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(Hint);
	applyPatches();
}

void Hint::applyPatches()
{
	// replace Omochao hint with custom
	static char const* STH2006_HINT = "STH2006_HINT";
	WRITE_MEMORY(0x11076F5, char*, STH2006_HINT);
	WRITE_MEMORY(0x11077C5, char*, STH2006_HINT);
	WRITE_JUMP(0x1107710, (void*)0x110771C);
	WRITE_JUMP(0x11077E0, (void*)0x11077EC);
}

Hint::~Hint()
{
	if (m_Data.m_HintFile)
	{
		m_Data.m_HintFile->Release();
	}

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
	char const* hintFile = "HintFile";
	m_Data.m_HintFile = Sonic::CParamTypeList::Create(&dummy, hintFile);
	m_Data.m_HintFile->m_pMember->m_DefaultValue = 1;
	m_Data.m_HintFile->m_pMember->m_pFuncData->m_ValueMax = 1;
	if (m_Data.m_HintFile)
	{
		m_Data.m_HintFile->AddRef();
	}
	in_rEditParam.CreateParamBase(m_Data.m_HintFile, hintFile);

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
	in_rEditParam.CreateParamInt(&m_Data.m_CharacterType, "CharacterType");
	in_rEditParam.CreateParamFloat(&m_Data.m_CollisionSize.x(), "Collision_Width");
	in_rEditParam.CreateParamFloat(&m_Data.m_CollisionSize.y(), "Collision_Height");
	in_rEditParam.CreateParamFloat(&m_Data.m_CollisionSize.z(), "Collision_Length");
	in_rEditParam.CreateParamBool(&m_Data.m_UseVoiceTime, "UseVoiceTime");
	in_rEditParam.CreateParamBase(Sonic::CParamTarget::Create(&m_Data.m_NextHintID), "NextHintID");
}

void Hint::SetAddUpdateUnit
(
	Sonic::CGameDocument* in_pGameDocument
)
{
	in_pGameDocument->AddUpdateUnit("0", this);
	in_pGameDocument->AddUpdateUnit("1", this);
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

	// play loop pfx
	auto const attachNode = m_spModel->GetNode("Hintring");
	m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, attachNode, "ef_hint", 1.0f);

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

	// kill if character doesn't match
	if (!IsCharacterMatch())
	{
		Kill();
		return;
	}

	// hint disabled in option
	uint32_t const optionFlags = *(uint32_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x18 });
	if ((optionFlags & 0x2) == 0)
	{
		Kill();
		return;
	}

	// cache .mst file and get subtitle entry
	auto const& file = m_Data.m_HintFile->m_pMember->m_DefaultValueName;
	if (!file.empty() && MstManager::RequestMst(file.c_str()))
	{
		m_entry = MstManager::GetSubtitle(file.c_str(), m_Data.m_HintName->m_pMember->m_DefaultValueName.c_str());
	}
	else
	{
		Kill();
	}
}

void Hint::GetObjectTriggerType
(
	hh::vector<uint32_t>& in_rTriggerTypeList
)
{
	in_rTriggerTypeList.push_back(4);
}

void Hint::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	// Non-time dependent update
	if (in_rUpdateInfo.Category == "1")
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
			}
		}
		return;
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
		times.push_back(std::stof(timesStr));
	}

	m_isPlaying = true;
	m_timer = SubtitleUI::addSubtitle(m_entry, times);

	if (m_Data.m_Type == (int)Type::Default)
	{
		ChangeState("Play");

		SharedPtrTypeless sfx;
		Common::ObjectPlaySound(this, 200600002, sfx);

		// play pfx
		auto const attachNode = m_spModel->GetNode("Hintring");
		m_pGlitterPlayer->PlayOneshot(attachNode, "ef_hint_play", 1.0f, 1);
	}
	else
	{
		// invisible ones kills immediately
		Kill();
	}
}

bool Hint::IsCharacterMatch()
{
	if (m_Data.m_CharacterType > 0)
	{
		S06DE_API::ModelType const modelType = S06DE_API::GetModelType();
		switch (modelType)
		{
		case S06DE_API::ModelType::None:
		case S06DE_API::ModelType::Sonic:
		case S06DE_API::ModelType::SonicElise:
			return m_Data.m_CharacterType == (int)CharacterType::Sonic;
		case S06DE_API::ModelType::Blaze:
			return m_Data.m_CharacterType == (int)CharacterType::Blaze;
		case S06DE_API::ModelType::Shadow:
			return m_Data.m_CharacterType == (int)CharacterType::Shadow;
		default:
			return false;
		}
	}

	return true;
}
