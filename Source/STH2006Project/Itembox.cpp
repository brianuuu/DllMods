#include "Itembox.h"
#include "UIContext.h"
#include "Application.h"
#include "Configuration.h"

float const c_10ringRadius = 0.57f;
float const c_1upRadius = 0.70f;

std::deque<ItemboxGUI> Itembox::m_guiData;
PDIRECT3DTEXTURE9 Itembox::m_item_5ring = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_10ring = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_20ring = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_1up = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_invin = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_speed = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_gauge = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_shield = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_fire = nullptr;

HOOK(void, __fastcall, Itembox_GetItem, 0xFFF810, uint32_t* This, void* Edx, void* message)
{
	uint32_t type = 0;
	ItemboxType enumType = IT_COUNT;

	switch (This[71])
	{
	default:	type = 2;	enumType = IT_1up;		break; // 1up
	case 1:		type = 4;	enumType = IT_invin;	break; // invincibility
	case 2:		type = 0;	enumType = IT_shield;	break; // shield
	case 3:		type = 8;	enumType = IT_fire;		break; // fire shield
	case 4:		type = 3;	enumType = IT_speed;	break; // speed shoes
	case 5:		type = 20;	enumType = IT_gauge;	break; // gauge up
	case 6:		type = 21;	enumType = IT_5ring;	break; // 5 ring
	case 7:		type = 22;	enumType = IT_10ring;	break; // 10 ring
	case 8:		type = 23;	enumType = IT_20ring;	break; // 20 ring
	}

	// Modify what passes to MsgGetItemType
	WRITE_MEMORY(0xFFF82F, uint32_t, type);

	// Draw ImGui
	Itembox::addItemToGui(enumType);

	originalItembox_GetItem(This, Edx, message);
}

struct MsgTakeObject
{
	INSERT_PADDING(0x10);
	uint32_t m_type;
};

HOOK(void, __fastcall, Itembox_MsgTakeObject, 0xE6DEC0, void* This, void* Edx, MsgTakeObject* message)
{
	// Set CPlayerSpeedStatePluginRingCountUp amount
	switch (message->m_type)
	{
	case 21: WRITE_MEMORY(0xE40DDF, uint32_t, 5); break;
	default: WRITE_MEMORY(0xE40DDF, uint32_t, 10); break;
	case 23: WRITE_MEMORY(0xE40DDF, uint32_t, 20); break;
	}

	if (message->m_type > 20)
	{
		message->m_type = 1;
	}

	originalItembox_MsgTakeObject(This, Edx, message);
}

HOOK(void, __fastcall, Itembox_MsgGetItemType, 0xE6D7D0, void* This, void* Edx, void* a2)
{
	uint32_t type = *(uint32_t*)((uint32_t)a2 + 16);
	if (type == 20)
	{
		// Gauge up
		float const maxBoost = Common::GetPlayerMaxBoost();
		float* currentBoost = Common::GetPlayerBoost();
		if (*currentBoost < maxBoost)
		{
			*currentBoost = maxBoost;
		}
	}
	else if (type >= 21 && type <= 23)
	{
		// 5,10,20 ring
		MsgTakeObject msgTakeObject = {};
		msgTakeObject.m_type = type;

		FUNCTION_PTR(void, __thiscall, processMsgTakeObject, 0xE6DEC0, void* This, void* pMessage);
		void* player = *(void**)((uint32_t)*PLAYER_CONTEXT + 0x110);
		processMsgTakeObject(player, &msgTakeObject);
	}

	originalItembox_MsgGetItemType(This, Edx, a2);
}

HOOK(void, __fastcall, Itembox_GetSuperRing, 0x11F2F10, uint32_t* This, void* Edx, void* message)
{
	Itembox::addItemToGui(ItemboxType::IT_10ring);
	originalItembox_GetSuperRing(This, Edx, message);
}

// Have to use ASM to prevent double playing sfx
void __declspec(naked) objItemPlaySfx()
{
	static uint32_t sub_D5D940 = 0xD5D940;
	static uint32_t returnAddress = 0xFFF9A8;
	__asm
	{
		mov     ecx, [esp + 14h]
		call	Itembox::playItemboxPfx
		call	Itembox::playItemboxSfx
		mov     esi, [esp + 14h]
		call	[sub_D5D940]
		jmp		[returnAddress]
	}
}

bool HandleHomingAttackMessageCommon(hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message)
{
	if (message.Is<Sonic::Message::MsgGetHomingAttackPriority>())
	{
		auto& msg = static_cast<Sonic::Message::MsgGetHomingAttackPriority&>(message);
		*msg.m_pPriority = 10;
		return true;
	}
	else if (message.Is<Sonic::Message::MsgGetHomingAttackPosition>())
	{
		auto& msg = static_cast<Sonic::Message::MsgGetHomingAttackPosition&>(message);
		auto* obj = static_cast<Sonic::CGameObject3D*>(This);
		*msg.m_pPosition = obj->m_spMatrixNodeTransform->m_Transform.m_Position;
		return true;
	}
}

void SendMsgDamageSuccess(Sonic::CGameObject3D* This, void* Edx, hh::fnd::Message& message)
{
	// Send MsgDamageSuccess back to sender
	This->SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamageSuccess>
		(
			This->m_spMatrixNodeTransform->m_Transform.m_Position, true, true, true, 8
		)
	);
}

HOOK(bool, __fastcall, Itembox_CStateLandJumpShortMsgDamageSuccess, 0x111CC20, Sonic::CGameObject3D* This, void* Edx, Sonic::Message::MsgDamageSuccess& message)
{
	if (message.m_DisableBounce) return false;
	return originalItembox_CStateLandJumpShortMsgDamageSuccess(This, Edx, message);
}

HOOK(bool, __fastcall, Itembox_CObjItemProcessMessage, 0xFFFD70, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
	if (flag && HandleHomingAttackMessageCommon(This, Edx, message))
	{
		return true;
	}

	return originalItembox_CObjItemProcessMessage(This, Edx, message, flag);
}

HOOK(void, __fastcall, Itembox_CObjItemMsgHitEventCollision, 0xFFF810, Sonic::CGameObject3D* This, void* Edx, hh::fnd::Message& message)
{
	SendMsgDamageSuccess(This, Edx, message);
	originalItembox_CObjItemMsgHitEventCollision(This, Edx, message);
}


HOOK(bool, __fastcall, Itembox_CObjSuperRingProcessMessage, 0x11F3680, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
	if (flag && HandleHomingAttackMessageCommon(This, Edx, message))
	{
		return true;
	}

	return originalItembox_CObjSuperRingProcessMessage(This, Edx, message, flag);
}

HOOK(void, __fastcall, Itembox_CObjSuperRingMsgHitEventCollision, 0x11F2F10, Sonic::CGameObject3D* This, void* Edx, hh::fnd::Message& message)
{
	SendMsgDamageSuccess(This, Edx, message);
	originalItembox_CObjSuperRingMsgHitEventCollision(This, Edx, message);
}

void Itembox::applyPatches()
{
	// Play itembox sfx for 1up and 10ring
	WRITE_MEMORY(0x011F2FE0, uint32_t, 4002032);
	WRITE_JUMP(0xFFF99F, objItemPlaySfx);

	// Set itembox radius
	WRITE_MEMORY(0x11F3353, float*, &c_10ringRadius);
	WRITE_MEMORY(0xFFF9E8, float*, &c_1upRadius);

	// Disable ef_ch_sns_yh1_ringget on 10ring
	WRITE_STRING(0x166E4CC, "");

	// Disable ef_ch_sng_yh1_1up after getting 1up
	WRITE_STRING(0x15E90DC, "");

	// Disable ef_ob_com_yh1_1up effect on 1up
	WRITE_JUMP(0xFFFA5E, (void*)0xFFFB15);

	// Disable Item always facing screen
	WRITE_NOP(0xFFF4E3, 3);

	// Handle homing lock-on for Item
	WRITE_MEMORY(0xFFFA3C, uint32_t, 0x1E0AF34);
	INSTALL_HOOK(Itembox_CObjItemProcessMessage);
	INSTALL_HOOK(Itembox_CObjItemMsgHitEventCollision);

	// Handle homing lock-on for SuperRing
	WRITE_MEMORY(0x11F336D, uint32_t, 0x1E0AF34);
	INSTALL_HOOK(Itembox_CObjSuperRingProcessMessage);
	INSTALL_HOOK(Itembox_CObjSuperRingMsgHitEventCollision);

	// Prevent bouncing off Itembox right as you land
	INSTALL_HOOK(Itembox_CStateLandJumpShortMsgDamageSuccess);

	//---------------------------------------
	// Inject new types to Item
	//---------------------------------------
	// Changes item model and effect
	static char const* itemNames[] = 
	{ 
		"cmn_obj_oneup_HD", 
		"cmn_obj_muteki_HD", // invincibility
		"cmn_obj_shield_HD",
		"cmn_obj_baria_HD",  // fire shield
		"cmn_obj_speed_HD",
		"cmn_obj_gaugeup_HD",
		"cmn_obj_5ring_HD",
		"cmn_obj_10ring_HD",
		"cmn_obj_20ring_HD"
	};
	WRITE_MEMORY(0xFFF566, char**, itemNames);
	INSTALL_HOOK(Itembox_GetItem);
	INSTALL_HOOK(Itembox_MsgGetItemType);
	INSTALL_HOOK(Itembox_MsgTakeObject);

	// Make CPlayerSpeedStatePluginRingCountUp get all rings immediately
	WRITE_NOP(0xE4155E, 2);
	WRITE_NOP(0xE415C8, 2);
	WRITE_MEMORY(0xE4157A, int, -1);

	// Force all item to use cmn_obj_oneup_HD skl and anm
	WRITE_NOP(0xFFF414, 6);
	WRITE_NOP(0xFFF593, 6);
	WRITE_MEMORY(0xFFF5D6, uint8_t, 0xB8, 0x3C, 0x46, 0x66, 0x01, 0x90, 0x90);
	WRITE_MEMORY(0xFFF62A, uint8_t, 0xB8, 0x3C, 0x46, 0x66, 0x01, 0x90, 0x90);

	// Prevent Disable Lives code conflict
	WRITE_MEMORY(0xFFF5D7, char*, itemNames[0]);
	WRITE_MEMORY(0xFFF62B, char*, itemNames[0]);

	// Draw GUI
	INSTALL_HOOK(Itembox_GetSuperRing);
}

void Itembox::playItemboxSfx()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 4002032, 0);
}

void __fastcall Itembox::playItemboxPfx(void* This)
{
	Common::ObjectCGlitterPlayerOneShot(This, "ef_ch_sns_yh1_1upget_s");
}

void Itembox::addItemToGui(ItemboxType type)
{
	if (type >= ItemboxType::IT_COUNT) return;

	// Initialize GUI data
	m_guiData.push_front(ItemboxGUI(type));
	
	// Only allow 5 to display at max
	if (m_guiData.size() > 5)
	{
		m_guiData.pop_back();
	}
}

bool Itembox::initTextures()
{
	std::wstring const dir = Application::getModDirWString();
	bool success = true;
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_5ring.dds").c_str(), &m_item_5ring);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_10ring.dds").c_str(), &m_item_10ring);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_20ring.dds").c_str(), &m_item_20ring);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_1up.dds").c_str(), &m_item_1up);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_invin.dds").c_str(), &m_item_invin);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_speed.dds").c_str(), &m_item_speed);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_gauge.dds").c_str(), &m_item_gauge);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_shield.dds").c_str(), &m_item_shield);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_fire.dds").c_str(), &m_item_fire);

	if (!success)
	{
		MessageBox(nullptr, TEXT("Failed to load assets for itembox icons, they may not display correctly"), TEXT("STH2006 Project"), MB_ICONWARNING);
	}

	return success;
}

void Itembox::draw()
{
	// At loading screen, clear all
	if (Common::IsAtLoadingScreen())
	{
		clearDraw();
		return;
	}

	// Remove frames that has timed out
	while (!m_guiData.empty() && m_guiData.back().m_frame > 200.0f)
	{
		m_guiData.pop_back();
	}

	// 0-8: zoom in
	// 9: zoom to size
	// 10-200: stay
	for (int i = m_guiData.size() - 1; i >= 0; i--)
	{
		ItemboxGUI& data = m_guiData[i];

		PDIRECT3DTEXTURE9* texture = nullptr;
		switch (data.m_type)
		{
		case IT_5ring:	 texture = &m_item_5ring;	break;
		case IT_10ring:	 texture = &m_item_10ring;	break;
		case IT_20ring:	 texture = &m_item_20ring;	break;
		case IT_1up:	 texture = &m_item_1up;		break;
		case IT_invin:	 texture = &m_item_invin;	break;
		case IT_speed:	 texture = &m_item_speed;	break;
		case IT_gauge:	 texture = &m_item_gauge;	break;
		case IT_shield:	 texture = &m_item_shield;	break;
		case IT_fire:	 texture = &m_item_fire;	break;
		}

		if (data.m_frame > 0.0f && texture)
		{
			static bool visible = true;
			ImGui::Begin((std::string("Itembox") + std::to_string(i)).c_str(), &visible, UIContext::m_hudFlags);
			{
				float sizeX = *BACKBUFFER_WIDTH * 192.0f / 1280.0f;
				float sizeY = *BACKBUFFER_HEIGHT * 192.0f / 720.0f;
				if (data.m_frame <= 8.0f)
				{
					float scale = data.m_frame / 8.0f * 1.12f;
					sizeX *= scale;
					sizeY *= scale;
				}
				else if (data.m_frame < 10.0f)
				{
					float scale = ((10.0f - data.m_frame) * 0.12f) + 1.0f;
					sizeX *= scale;
					sizeY *= scale;
				}

				float targetPos = 0.5f - (0.091f * i) * (Configuration::m_using06HUD ? 1.0f : -1.0f);
				data.m_pos += (targetPos - data.m_pos) * 0.15f * Application::getHudDeltaTime() * 60.0f;
				float posX = floorf(*BACKBUFFER_WIDTH * data.m_pos - sizeX * 0.5f - 8.0f);
				float posY = floorf(*BACKBUFFER_HEIGHT * 0.847f - sizeX * 0.5f - 8.0f);
				ImGui::SetWindowFocus();
				ImGui::SetWindowPos(ImVec2(posX, posY));
				ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
				ImGui::Image(*texture, ImVec2(sizeX, sizeY));
			}
			ImGui::End();
		}

		data.m_frame += Application::getHudDeltaTime() * 60.0f;
	}
}

void Itembox::clearDraw()
{
	m_guiData.clear();
}
