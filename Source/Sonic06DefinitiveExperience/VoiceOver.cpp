#include "VoiceOver.h"

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
		jmp[objRainbowRingVoiceReturnAddress]
	}
}

void VoiceOver::applyPatches()
{
	// Play rainbow ring voice
	WRITE_JUMP(0x115A8EC, objRainbowRingVoice);
}

void VoiceOver::playItemboxVoice()
{
	static SharedPtrTypeless soundHandle;
	soundHandle.reset();
	Common::SonicContextPlaySound(soundHandle, 3002021, 0);
}

void VoiceOver::playRainbowRingVoice()
{
	static SharedPtrTypeless soundHandle;
	soundHandle.reset();
	Common::SonicContextPlaySound(soundHandle, 3002022, 0);
}
