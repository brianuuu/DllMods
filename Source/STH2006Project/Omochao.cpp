#include "Omochao.h"
#include "Application.h"

HOOK(int, __fastcall, Omochao_MsgNotifyObjectEvent, 0x114FB60, void* This, void* Edx, uint32_t a2)
{
	uint32_t* pEvent = (uint32_t*)(a2 + 16);
	S06DE_API::ModelType modelType = S06DE_API::GetModelType();
	switch (*pEvent)
	{
	case 50:
	{
		// Sonic specific dialogs (Super Sonic in Elise is also Sonic)
		if ((Common::IsPlayerSuper() && S06DE_API::GetModelType() == S06DE_API::ModelType::SonicElise)
		|| modelType == S06DE_API::ModelType::Sonic || modelType == S06DE_API::ModelType::None)
		{
			*pEvent = 6;
		}
		break;
	}
	case 51:
	{
		// Elise specific dialogs
		if (!Common::IsPlayerSuper() && S06DE_API::GetModelType() == S06DE_API::ModelType::SonicElise)
		{
			*pEvent = 6;
		}
		break;
	}
	case 52:
	{
		// Sonic OR Elise specific dialogs
		if (modelType == S06DE_API::ModelType::None || modelType == S06DE_API::ModelType::Sonic || S06DE_API::GetModelType() == S06DE_API::ModelType::SonicElise)
		{
			*pEvent = 6;
		}
		break;
	}
	case 53:
	{
		// Blaze specific dialogs
		if (S06DE_API::GetModelType() == S06DE_API::ModelType::Blaze)
		{
			*pEvent = 6;
		}
		break;
	}
	default: break;
	}

	return originalOmochao_MsgNotifyObjectEvent(This, Edx, a2);
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
}
