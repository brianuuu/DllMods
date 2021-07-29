#include "VoiceOver.h"

HOOK(int, __fastcall, VoiceOver_CSonicStateHomingAttackBegin, 0x1232040, void* This)
{
	VoiceOver::playHomingAttackVoice();
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

// Play rainbow ring voice
uint32_t objRainbowRingVoiceReturnAddress = 0x115A8F2;
void __declspec(naked) objRainbowRingVoice()
{
	__asm
	{
		push	esi
		call	VoiceOver::playRainbowRingVoice
		pop		esi

		mov     eax, [esi + 0B8h]
		jmp		[objRainbowRingVoiceReturnAddress]
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
	WRITE_MEMORY(0x11184E4, uint32_t, 3002000);
	WRITE_MEMORY(0x1118512, uint32_t, 3002000);
	INSTALL_HOOK(VoiceOver_CSonicStateHomingAttackAfterBegin);

	// Add jump voices to walljump
	INSTALL_HOOK(VoiceOver_CSonicStateWallJumpBegin);
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
