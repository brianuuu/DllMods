#include "Itembox.h"
#include "UIContext.h"
#include "Application.h"
#include "Configuration.h"

std::deque<ItemboxGUI> Itembox::m_guiData;
IUnknown* Itembox::m_item_5ring = nullptr;
IUnknown* Itembox::m_item_10ring = nullptr;
IUnknown* Itembox::m_item_20ring = nullptr;
IUnknown* Itembox::m_item_1up = nullptr;
IUnknown* Itembox::m_item_invin = nullptr;
IUnknown* Itembox::m_item_speed = nullptr;
IUnknown* Itembox::m_item_gauge = nullptr;
IUnknown* Itembox::m_item_shield = nullptr;
IUnknown* Itembox::m_item_fire = nullptr;

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

	return false;
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

float const c_invincibleTime = 16.8f;
HOOK(void, __fastcall, Itembox_CPlayerSpeedStatePluginUnbeatenAdvance, 0x11DD640, Sonic::Player::CPlayerSpeedContext::CStateSpeedBase* This)
{
	if (This->m_Time > c_invincibleTime)
	{
		This->m_Field1C = 1;
	}
}

HOOK(void, __fastcall, Itembox_CPlayerSpeedStatePluginHighSpeedBegin, 0x111B660, Sonic::Player::CPlayerSpeedContext::CStateSpeedBase* This)
{
	originalItembox_CPlayerSpeedStatePluginHighSpeedBegin(This);

	// Play Invincible2 music
	Common::PlayBGM((char*)0x15F3130, 0.0f);
	*(Hedgehog::Base::CSharedString*)((uint32_t)This + 0x60) = (char*)0x15F3130;
}

HOOK(void, __fastcall, Itembox_CPlayerSpeedStatePluginHighSpeedAdvance, 0x111B600, Sonic::Player::CPlayerSpeedContext::CStateSpeedBase* This)
{
	auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
	if (This->m_Time > c_invincibleTime || context->StateFlag(eStateFlag_Dead))
	{
		This->m_Field1C = 1;
	}
}

void Itembox::applyPatches()
{
	//---------------------------------------
	// Common
	//---------------------------------------
	// Play itembox sfx for 1up and 10ring
	WRITE_MEMORY(0x11F2FE0, uint32_t, 200600022);
	WRITE_JUMP(0xFFF99F, objItemPlaySfx);

	// Set itembox radius
	static float const c_itemboxRadius = 0.57f;
	WRITE_MEMORY(0x11F3353, float*, &c_itemboxRadius);
	WRITE_MEMORY(0xFFF9E8, float*, &c_itemboxRadius);

	// Make CPlayerSpeedStatePluginRingCountUp get all rings immediately
	WRITE_NOP(0xE4155E, 2);
	WRITE_NOP(0xE415C8, 2);
	WRITE_MEMORY(0xE4157A, int, -1);

	//---------------------------------------
	// SuperRing
	//---------------------------------------
	// Disable ef_ch_sns_yh1_ringget on 10ring
	WRITE_STRING(0x166E4CC, "");

	// Play itembox pfx
	WRITE_STRING(0x166E4E8, "ef_itembox");
	
	// Handle homing lock-on for SuperRing
	WRITE_MEMORY(0x11F336D, uint32_t, 0x1E0AF34);
	INSTALL_HOOK(Itembox_CObjSuperRingProcessMessage);
	INSTALL_HOOK(Itembox_CObjSuperRingMsgHitEventCollision);

	// Stop auto face camera
	WRITE_NOP(0x11F2BD0, 6); 

	// Force replace model
	WRITE_STRING(0x166E498, "cmn_superring");
	WRITE_STRING(0x166E4B0, "cmn_itembox");

	// Match itembox spin rate
	static double const c_superring_spinRate = -0.8 * PI;
	WRITE_MEMORY(0x11F2A6A, double*, &c_superring_spinRate);

	// Draw GUI
	INSTALL_HOOK(Itembox_GetSuperRing);

	//---------------------------------------
	// Item
	//---------------------------------------
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

	// Prevent bouncing off Itembox right as you land
	INSTALL_HOOK(Itembox_CStateLandJumpShortMsgDamageSuccess);

	// Fix invincibility time
	INSTALL_HOOK(Itembox_CPlayerSpeedStatePluginUnbeatenAdvance);

	// Fix speed shoes music and time
	WRITE_JUMP(0x111B7E7, (void*)0x111B873);
	INSTALL_HOOK(Itembox_CPlayerSpeedStatePluginHighSpeedBegin);
	INSTALL_HOOK(Itembox_CPlayerSpeedStatePluginHighSpeedAdvance);

	// Inject new types to Item
	static char const* itemNames[] = 
	{ 
		"cmn_itembox_oneup", 
		"cmn_itembox_muteki", // invincibility
		"cmn_itembox_shield",
		"cmn_itembox_baria",  // fire shield
		"cmn_itembox_speed",
		"cmn_itembox_gaugeup",
		"cmn_itembox_ring5",
		"cmn_itembox_ring10",
		"cmn_itembox_ring20"
	};
	WRITE_MEMORY(0xFFF566, char**, itemNames);
	WRITE_MEMORY(0xFFF5D9, char**, itemNames);
	WRITE_MEMORY(0xFFF62D, char**, itemNames);
	INSTALL_HOOK(Itembox_GetItem);
	INSTALL_HOOK(Itembox_MsgGetItemType);
	INSTALL_HOOK(Itembox_MsgTakeObject);

	// Force all item to use cmn_itembox_oneup skl and anm
	WRITE_NOP(0xFFF414, 6);
	WRITE_NOP(0xFFF593, 6);
	WRITE_MEMORY(0xFFF5D0, uint8_t, 0x31, 0xD2, 0x90, 0x90, 0x90, 0x90);
	WRITE_MEMORY(0xFFF624, uint8_t, 0x31, 0xD2, 0x90, 0x90, 0x90, 0x90);

	// Barrier use actual pfx and not Aqua shield's
	WRITE_MEMORY(0x119E7CD, uint32_t, 0x1E63F5C);
}

void Itembox::playItemboxSfx()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 200600022, 0);
}

void __fastcall Itembox::playItemboxPfx(void* This)
{
	Common::ObjectCGlitterPlayerOneShot(This, "ef_itembox");
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

		IUnknown** texture = nullptr;
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
