#include "Itembox.h"
#include "NextGenPhysics.h"

// Have to use ASM to prevent double playing sfx
void __declspec(naked) objItemPlaySfx()
{
	static uint32_t returnAddress = 0xFFF9A8;
	static uint32_t sub_D5D940 = 0xD5D940;
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
	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	if (context && message.m_SenderActorID != context->m_pPlayer->m_ActorID)
	{
		Common::SonicContextGetItemType(2); // 1-up
	}

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
	// Play itembox and voice sfx for 10ring and 1up
	WRITE_MEMORY(0x11F2FE0, uint32_t, 4002032);
	WRITE_JUMP(0xFFF99F, objItemPlaySfx);

	// Set itembox radius
	static float const c_itemboxRadius = 0.57f;
	WRITE_MEMORY(0x11F3353, float*, &c_itemboxRadius);
	WRITE_MEMORY(0xFFF9E8, float*, &c_itemboxRadius);

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
