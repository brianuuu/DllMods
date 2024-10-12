#include "VoiceOver.h"
#include "NextGenShadow.h"
#include "Configuration.h"

HOOK(int, __fastcall, VoiceOver_CSonicStateHomingAttackBegin, 0x1232040, hh::fnd::CStateMachineBase::CStateBase* This)
{
	auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
	bool disableVoice = NextGenShadow::m_chaosBoostLevel > 0 && context->m_HomingAttackTargetActorID;
	
	if (!disableVoice)
	{
		VoiceOver::playHomingAttackVoice();
	}

	return originalVoiceOver_CSonicStateHomingAttackBegin(This);
}

HOOK(void, __fastcall, VoiceOver_CSonicStateWallJumpBegin, 0x11BC9E0, void* This)
{
	VoiceOver::playJumpVoice();
	originalVoiceOver_CSonicStateWallJumpBegin(This);
}

HOOK(int32_t*, __fastcall, VoiceOver_CSonicStateHomingAttackAfterBegin, 0x1118300, void* This)
{
	// Play jump sfx after homing attack hit something
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 2002027, 1);
	return originalVoiceOver_CSonicStateHomingAttackAfterBegin(This);
}

HOOK(void, __fastcall, VoiceOver_SuperRingMsgHitEventCollision, 0x11F2F10, void* This, void* Edx, void* a2)
{
	VoiceOver::playItemboxVoice();
	originalVoiceOver_SuperRingMsgHitEventCollision(This, Edx, a2);
}

HOOK(void, __fastcall, VoiceOver_MsgGetItemType, 0xE6D7D0, void* This, void* Edx, void* a2)
{
	// This function also plays for Modern 1up
	switch (*(uint32_t*)((uint32_t)a2 + 16))
	{
	case 5: // Board
	case 6: // Rocket Wisp
	case 7: // Spike Wisp
	case 8: // Flame Barrier
	case 9: // Aqua Barrier
	case 10: // Thunder Barrier
	case 11:
	case 12:
	case 13: // Power stomp
	case 14:
	case 19:
		break;
	default:
		VoiceOver::playItemboxVoice();
		break;
	}
	originalVoiceOver_MsgGetItemType(This, Edx, a2);
}

// Play rainbow ring voice
void __declspec(naked) objRainbowRingVoice()
{
	static uint32_t returnAddress = 0x115A8F2;
	__asm
	{
		push	esi
		call	VoiceOver::playRainbowRingVoice
		pop		esi

		mov     eax, [esi + 0B8h]
		jmp		[returnAddress]
	}
}

void VoiceOver::applyPatches()
{
	// Play rainbow ring voice
	WRITE_JUMP(0x115A8EC, objRainbowRingVoice);

	// Disable trick voices
	WRITE_MEMORY(0xE4B684, int, -1);

	// Change trick finish voice (and use sfx player instead)
	WRITE_MEMORY(0xE4B8D7, uint8_t, 0x8B, 0x50, 0x74, 0x90, 0x90, 0x90);
	WRITE_MEMORY(0xE4B8E0, uint32_t, 3002025);

	// Add homing attack voices
	INSTALL_HOOK(VoiceOver_CSonicStateHomingAttackBegin);

	// Change homing attack after voices to jump voices
	// Shadow with custom moveset handles voice itself
	if (Configuration::m_model != Configuration::ModelType::Shadow || !Configuration::m_characterMoveset)
	{
		WRITE_MEMORY(0x11184E4, uint32_t, 3002000);
		WRITE_MEMORY(0x1118512, uint32_t, 3002000);
		INSTALL_HOOK(VoiceOver_CSonicStateHomingAttackAfterBegin);
	}

	// Add jump voices to walljump
	INSTALL_HOOK(VoiceOver_CSonicStateWallJumpBegin);

	// Play itembox voices
	INSTALL_HOOK(VoiceOver_SuperRingMsgHitEventCollision);
	INSTALL_HOOK(VoiceOver_MsgGetItemType);
}

void VoiceOver::playJumpVoice()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlayVoice(soundHandle, 3002000, 0);
}

void VoiceOver::playItemboxVoice()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 3002021, 0);
}

void VoiceOver::playRainbowRingVoice()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 3002022, 0);
}

void VoiceOver::playHomingAttackVoice()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 3002023, 0);
}

void VoiceOver::playStageCompleteVoice()
{
	// Statuc since sonic context sound doesn't work at result screen
	static SharedPtrTypeless soundHandle;
	Common::PlaySoundStatic(soundHandle, 3002019);
}
