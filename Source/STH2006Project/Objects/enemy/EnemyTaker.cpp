#include "EnemyTaker.h"
#include "DarkSphere.h"

HOOK(void, __fastcall, EnemyTaker_InitializeEditParam, 0xBA02B0, uint32_t This, void* Edx, Sonic::CEditParam& in_rEditParam)
{
	originalEnemyTaker_InitializeEditParam(This, Edx, in_rEditParam);

	EnemyTaker* spEnemyTaker = (EnemyTaker*)(This - 0x100);
	spEnemyTaker->m_isDark = false;
	in_rEditParam.CreateParamBool(&spEnemyTaker->m_isDark, "IsDark");
}

void __declspec(naked) EnemyTaker_SetModel()
{
	static uint32_t returnAddress = 0xBA007C;
	static char const* enm_taker_HD = "enm_taker_HD";
	static char const* enm_daker_HD = "enm_daker_HD";
	__asm
	{
		cmp     byte ptr [esi + 5A0h], 0
		jz		original
		push	enm_daker_HD
		jmp		[returnAddress]

		original:
		push	enm_taker_HD
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyTaker_SetMaterial()
{
	static uint32_t returnAddress = 0xBA00AA;
	static char const* enm_taker_body = "enm_taker_body";
	static char const* enm_daker_body = "enm_daker_body";
	__asm
	{
		cmp     byte ptr [esi + 5A0h], 0
		jz		original
		push	enm_daker_body
		jmp		[returnAddress]

		original:
		push	enm_taker_body
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyTaker_SetAnim()
{
	static uint32_t returnAddress = 0xBA00CE;
	static char const* enm_taker_body_0000 = "enm_taker_body-0000";
	static char const* enm_daker_body_0000 = "enm_daker_body-0000";
	__asm
	{
		cmp     byte ptr [esi + 5A0h], 0
		jz		original
		push	enm_daker_body_0000
		jmp		[returnAddress]

		original:
		push	enm_taker_body_0000
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyTaker_SetMuzzle()
{
	static uint32_t returnAddress = 0xBA2A67;
	static char const* ef_en_clr_yh2_muzzle = "ef_en_clr_yh2_muzzle";
	static char const* ef_en_drk_yh2_muzzle = "ef_en_drk_yh2_muzzle";
	__asm
	{
		cmp     byte ptr [ebx + 5A0h], 0
		jz		original
		push	ef_en_drk_yh2_muzzle
		jmp		[returnAddress]

		original:
		push	ef_en_clr_yh2_muzzle
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyTaker_SetThrowSfx()
{
	static uint32_t returnAddress = 0xBA2A59;
	__asm
	{
		cmp     byte ptr [ebx + 5A0h], 0
		jz		original
		push	5162008
		jmp		[returnAddress]

		original:
		push	5162006
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyTaker_SetProjectileData()
{
	static uint32_t returnAddress = 0xBA1B23;
	static char const* enm_togeball = "enm_togeball";
	static char const* ef_mephiles_sphere_s = "ef_mephiles_sphere_s";
	__asm
	{
		cmp     byte ptr [ebx + 5A0h], 0
		jz		original
		push	ef_mephiles_sphere_s
		lea     ecx, [esp + 68h] // m_EffectName
		jmp		[returnAddress]

		original:
		push	enm_togeball
		lea     ecx, [esp + 64h] // m_ModelName
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyTaker_SetExplosionData()
{
	static uint32_t returnAddress = 0xBA1BBC;
	__asm
	{
		cmp     byte ptr [ebx + 5A0h], 0
		jz		original
		lea		eax, m_darkSphereExplosionData
		mov		dword ptr [esp + 0xDC], eax
		jmp		[returnAddress]

		original:
		mov		dword ptr [esp + 0x9C], 0x3F800000 // m_ParticleScale = 0.5
		mov		dword ptr [esp + 0xDC], 0x1E671B4
		jmp		[returnAddress]
	}
}

void EnemyTaker::applyPatches()
{
	// increase alloc size
	WRITE_MEMORY(0xBA25A9, uint32_t, c_size + 4); // 2D
	WRITE_MEMORY(0xBA23E6, uint32_t, c_size + 4); // 3D

	// change model
	INSTALL_HOOK(EnemyTaker_InitializeEditParam);
	WRITE_JUMP(0xBA0077, EnemyTaker_SetModel);
	WRITE_JUMP(0xBA00A5, EnemyTaker_SetMaterial);
	WRITE_JUMP(0xBA00C9, EnemyTaker_SetAnim);
	WRITE_MEMORY(0x16F8C54 + 0xC8, void*, AddCallback);

	// change effect
	WRITE_JUMP(0xBA2A62, EnemyTaker_SetMuzzle);

	// set fireball data
	WRITE_JUMP(0xBA2A54, EnemyTaker_SetThrowSfx);
	WRITE_JUMP(0xBA1B1A, EnemyTaker_SetProjectileData);
	WRITE_JUMP(0xBA1BB1, EnemyTaker_SetExplosionData);
	WRITE_MEMORY(0x12B5D2F, uint32_t, 5162007);
	WRITE_MEMORY(0xBA1AE7, uint32_t, 0x1704138); // radius 0.7 -> 0.2

	// set dark monsters dead effect
	WRITE_STRING(0x1577920, "ef_en_drk_yh2_dead");
}

void __fastcall EnemyTaker::AddCallback
(
	EnemyTaker* This, void*, void*
)
{
	if (This->m_isDark)
	{
		This->m_energyAmount = 2;

		// set IsCrisisCityEnemy to 2
		uint32_t pAimTargetParams = *(uint32_t*)((uint32_t)This + 0x11C);
		*(int*)(pAimTargetParams + 0x28) = 2;
	}

	// can only construct in runtime?
	if (!m_darkSphereExplosionData.m_init)
	{
		m_darkSphereExplosionData.m_effectName = "ef_mephiles_spherebomb_s";
		m_darkSphereExplosionData.m_init = true;
	}
}
