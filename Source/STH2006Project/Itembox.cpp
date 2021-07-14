#include "Itembox.h"

float const c_itemboxRadius = 0.56f;

// Have to use ASM to prevent double playing sfx
uint32_t sub_D5FD10 = 0xD5FD10;
uint32_t objItemPlaySfxReturnAddress = 0xFFF9AF;
void __declspec(naked) objItemPlaySfx()
{
	__asm
	{
		call	[sub_D5FD10]
		call	Itembox::playItemboxSfx
		jmp		[objItemPlaySfxReturnAddress]
	}
}

void Itembox::applyPatches()
{
	// Play itembox sfx for 1up and 10ring
	WRITE_MEMORY(0x011F2FE0, uint32_t, 4002032);
	WRITE_JUMP(0xFFF9AA, objItemPlaySfx);

	// Set itembox radius
	WRITE_MEMORY(0x11F3353, float*, &c_itemboxRadius);
	WRITE_MEMORY(0xFFF9E8, float*, &c_itemboxRadius);
}

void Itembox::playItemboxSfx()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 4002032, 0);
}