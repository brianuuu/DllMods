#include "Omochao.h"

#include "System/Application.h"

enum HintType
{
	HT_Sonic = 50,
	HT_Elise,
	HT_SonicElise,
	HT_Blaze,

	HT_LAST,
	HT_FIRST = 50
};

bool DoesHintTypeMatch(HintType type)
{
	S06DE_API::ModelType modelType = S06DE_API::GetModelType();
	switch (type)
	{
	case HT_Sonic:
	{
		// Sonic specific dialogs (Super Sonic in Elise is also Sonic)
		if ((Common::IsPlayerSuper() && S06DE_API::GetModelType() == S06DE_API::ModelType::SonicElise)
		 || modelType == S06DE_API::ModelType::Sonic || modelType == S06DE_API::ModelType::None)
		{
			return true;
		}
		break;
	}
	case HT_Elise:
	{
		// Elise specific dialogs
		if (!Common::IsPlayerSuper() && S06DE_API::GetModelType() == S06DE_API::ModelType::SonicElise)
		{
			return true;
		}
		break;
	}
	case HT_SonicElise:
	{
		// Sonic OR Elise specific dialogs
		if (modelType == S06DE_API::ModelType::None || modelType == S06DE_API::ModelType::Sonic || S06DE_API::GetModelType() == S06DE_API::ModelType::SonicElise)
		{
			return true;
		}
		break;
	}
	case HT_Blaze:
	{
		// Blaze specific dialogs
		if (S06DE_API::GetModelType() == S06DE_API::ModelType::Blaze)
		{
			return true;
		}
		break;
	}
	}

	return false;
}

HOOK(int, __fastcall, Omochao_MsgNotifyObjectEvent, 0x114FB60, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
{
	bool subtitleEnabled = (*(uint8_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x18 })) & 0x2;
	if (!subtitleEnabled)
	{
		return originalOmochao_MsgNotifyObjectEvent(This, Edx, message);
	}

	if (DoesHintTypeMatch((HintType)message.m_Event))
	{
		message.m_Event = 6;
	}

	return originalOmochao_MsgNotifyObjectEvent(This, Edx, message);
}

HOOK(void, __fastcall, Omochao_MsgNotifyObjectEvent_CObjectPhysics, 0xEA4F50, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
{
	if (message.m_Event >= HT_FIRST && message.m_Event < HT_LAST)
	{
		// Destroy this object physics if not matching character
		if (!DoesHintTypeMatch((HintType)message.m_Event))
		{
			Common::fDestroyGameObject(This);
			return;
		}
	}

	originalOmochao_MsgNotifyObjectEvent_CObjectPhysics(This, Edx, message);
}

HOOK(int, __fastcall, Omochao_MsgNotifyObjectEvent_CEventCollision, 0xFEA400, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
{
	if (message.m_Event >= HT_FIRST && message.m_Event < HT_LAST)
	{
		// Disable event collision if not matching character
		if (!DoesHintTypeMatch((HintType)message.m_Event))
		{
			Common::fDestroyGameObject(This);
			return true;
		}
	}

	return originalOmochao_MsgNotifyObjectEvent_CEventCollision(This, Edx, message);
}

void Omochao::applyPatches()
{
	INSTALL_HOOK(Omochao_MsgNotifyObjectEvent);

	// Force different omochao model in HUB
	WRITE_STRING(0x161B4C0, "chr_omocha2_HD");
	WRITE_STRING(0x161B4D0, "chr_omocha2_HD");
	//WRITE_STRING(0x1667BDC, "chr_omocha2_HD");
	//WRITE_STRING(0x1667BEC, "chr_omocha2_HD");

	// Don't follow Sonic in stages
	WRITE_NOP(0x461724, 2);

	// Disable omochao pfx
	WRITE_STRING(0x16686E4, "");
	WRITE_STRING(0x166884C, "");
	WRITE_STRING(0x1668894, "");
	WRITE_STRING(0x1668A94, "");

	// Disable omochao sfx
	WRITE_MEMORY(0x460DD7, int, -1);
	WRITE_MEMORY(0x4627FC, int, -1);
	WRITE_MEMORY(0x462B98, int, -1);
	WRITE_MEMORY(0x462C75, int, -1);
	WRITE_MEMORY(0x1150999, int, -1);
	WRITE_MEMORY(0x1151AC4, int, -1);

	// Don't destroy omochao even if subtitle is disabled
	WRITE_NOP(0x1154C1B, 2);
	WRITE_NOP(0x10122AB, 6);

	// cmn_hint for characters
	INSTALL_HOOK(Omochao_MsgNotifyObjectEvent_CObjectPhysics);
	INSTALL_HOOK(Omochao_MsgNotifyObjectEvent_CEventCollision);
}
