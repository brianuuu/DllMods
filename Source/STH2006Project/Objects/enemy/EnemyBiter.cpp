#include "EnemyBiter.h"

HOOK(void, __fastcall, EnemyBiter_InitializeEditParam, 0xB844C0, uint32_t This, void* Edx, Sonic::CEditParam& in_rEditParam)
{
	originalEnemyBiter_InitializeEditParam(This, Edx, in_rEditParam);

	EnemyBiter* spEnemyBiter = (EnemyBiter*)(This - 0x100);
	spEnemyBiter->m_isDark = false;
	in_rEditParam.CreateParamBool(&spEnemyBiter->m_isDark, "IsDark");
}

void __declspec(naked) EnemyBiter_SetModel()
{
	static uint32_t returnAddress = 0xB8497F;
	static char const* enm_biter_HD = "enm_biter_HD";
	static char const* enm_diter_HD = "enm_diter_HD";
	__asm
	{
		cmp     byte ptr [esi + 270h], 0
		jz		original
		push	enm_diter_HD
		jmp		[returnAddress]

		original:
		push	enm_biter_HD
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyBiter_SetBody()
{
	static uint32_t returnAddress = 0xB849AD;
	static char const* enm_biter_HD_body = "enm_biter_HD_body";
	static char const* enm_diter_HD_body = "enm_diter_HD_body";
	__asm
	{
		cmp     byte ptr [esi + 270h], 0
		jz		original
		push	enm_diter_HD_body
		jmp		[returnAddress]

		original:
		push	enm_biter_HD_body
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyBiter_SetMgm()
{
	static uint32_t returnAddress = 0xB849D1;
	static char const* enm_biter_HD_mgm = "enm_biter_HD_mgm";
	static char const* enm_diter_HD_mgm = "enm_diter_HD_mgm";
	__asm
	{
		cmp     byte ptr [esi + 270h], 0
		jz		original
		push	enm_diter_HD_mgm
		jmp		[returnAddress]

		original:
		push	enm_biter_HD_mgm
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyBiter_SetAnim()
{
	static uint32_t returnAddress = 0xB849F5;
	static char const* enm_biter_HD_mgm_0000 = "enm_biter_HD_mgm-0000";
	static char const* enm_diter_HD_mgm_0000 = "enm_diter_HD_mgm-0000";
	__asm
	{
		cmp     byte ptr [esi + 270h], 0
		jz		original
		push	enm_diter_HD_mgm_0000
		jmp		[returnAddress]

		original:
		push	enm_biter_HD_mgm_0000
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyBiter_SetOmen()
{
	static uint32_t returnAddress = 0x60D641;
	static char const* ef_en_btr_yh2_fire_omen = "ef_en_btr_yh2_fire_omen";
	static char const* ef_en_drk_yh2_fire_omen = "ef_en_drk_yh2_fire_omen";
	__asm
	{
		cmp     byte ptr [eax + 270h], 0
		jz		original
		push	ef_en_drk_yh2_fire_omen
		jmp		[returnAddress]

		original:
		push	ef_en_btr_yh2_fire_omen
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyBiter_SetFire()
{
	static uint32_t returnAddress = 0xB84045;
	static char const* ef_en_btr_yh2_fire = "ef_en_btr_yh2_fire";
	static char const* ef_en_drk_yh2_fire = "ef_en_drk_yh2_fire";
	__asm
	{
		cmp     byte ptr [esi + 270h], 0
		jz		original
		push	ef_en_drk_yh2_fire
		jmp		[returnAddress]

		original:
		push	ef_en_btr_yh2_fire
		jmp		[returnAddress]
	}
}

void EnemyBiter::applyPatches()
{
	// increase alloc size
	WRITE_MEMORY(0xB866D3, uint32_t, c_size + 4); // 2D
	WRITE_MEMORY(0xB86516, uint32_t, c_size + 4); // 3D

	// change model
	INSTALL_HOOK(EnemyBiter_InitializeEditParam);
	WRITE_JUMP(0xB8497A, EnemyBiter_SetModel);
	WRITE_JUMP(0xB849A8, EnemyBiter_SetBody);
	WRITE_JUMP(0xB849CC, EnemyBiter_SetMgm);
	WRITE_JUMP(0xB849F0, EnemyBiter_SetAnim);
	WRITE_MEMORY(0x16FAD14 + 0xC8, void*, AddCallback);

	// change effect
	WRITE_JUMP(0x60D63C, EnemyBiter_SetOmen);
	WRITE_JUMP(0xB84040, EnemyBiter_SetFire);

	// Add radius 0.1 -> 0.3
	WRITE_MEMORY(0xB86028, uint32_t, 0x1703D70);
}

void __fastcall EnemyBiter::AddCallback
(
	EnemyBiter* This, void*, void*
)
{
	if (This->m_isDark)
	{
		This->m_energyAmount = 2;

		// set IsCrisisCityEnemy to 2
		uint32_t pAimTargetParams = *(uint32_t*)((uint32_t)This + 0x11C);
		*(int*)(pAimTargetParams + 0x28) = 2;
	}
}
