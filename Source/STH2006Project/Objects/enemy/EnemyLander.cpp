#include "EnemyLander.h"

HOOK(void, __fastcall, EnemyLander_InitializeEditParam, 0xBCDA70, uint32_t This, void* Edx, Sonic::CEditParam& in_rEditParam)
{
	originalEnemyLander_InitializeEditParam(This, Edx, in_rEditParam);

	EnemyLander* spEnemyLander = (EnemyLander*)(This - 0x100);
	spEnemyLander->m_isCommander = false;
	in_rEditParam.CreateParamBool(&spEnemyLander->m_isCommander, "IsCommander");
}

HOOK(int*, __fastcall, EnemyLander_SpawnBrk, 0xBCBBC0, EnemyLander* This)
{
	int* result = originalEnemyLander_SpawnBrk(This);

	static Hedgehog::Base::CSharedString enm_lander_brk = "enm_lander_brk";
	static Hedgehog::Base::CSharedString enm_cander_brk = "enm_cander_brk";
	static Hedgehog::Base::CSharedString enm_lander_brk_ = "enm_lander_brk_";
	static Hedgehog::Base::CSharedString enm_cander_brk_ = "enm_cander_brk_";
	if (This->m_isCommander)
	{
		WRITE_MEMORY(0x1E77894, char*, enm_cander_brk.get());
		WRITE_MEMORY(0x1E77898, char*, enm_cander_brk_.get());
	}
	else
	{
		WRITE_MEMORY(0x1E77894, char*, enm_lander_brk.get());
		WRITE_MEMORY(0x1E77898, char*, enm_lander_brk_.get());
	}

	return result;
}

void __declspec(naked) EnemyLander_SetModel()
{
	static uint32_t returnAddress = 0xBCF054;
	static char const* enm_lander_HD = "enm_lander_HD";
	static char const* enm_cander_HD = "enm_cander_HD";
	__asm
	{
		mov		eax, [ebx + 280h]
		test	al, al
		jz		original
		push	enm_cander_HD
		jmp		[returnAddress]

		original:
		push	enm_lander_HD
		jmp		[returnAddress]
	}
}

void EnemyLander::applyPatches()
{
	// increase alloc size
	WRITE_MEMORY(0xBCE979, uint32_t, c_size + 4); // 2D
	WRITE_MEMORY(0xBCE7B6, uint32_t, c_size + 4); // 3D

	// increase radius
	WRITE_MEMORY(0xBCE449, uint32_t, 0x1574644); // 0.8 -> 1.0

	INSTALL_HOOK(EnemyLander_InitializeEditParam);
	INSTALL_HOOK(EnemyLander_SpawnBrk);
	WRITE_JUMP(0xBCF04F, EnemyLander_SetModel);
}
