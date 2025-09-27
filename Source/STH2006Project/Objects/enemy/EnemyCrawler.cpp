#include "EnemyCrawler.h"

HOOK(void, __fastcall, EnemyCrawler_InitializeEditParam, 0xB97300, uint32_t This, void* Edx, Sonic::CEditParam& in_rEditParam)
{
	originalEnemyCrawler_InitializeEditParam(This, Edx, in_rEditParam);

	EnemyCrawler* spEnemyCrawler = (EnemyCrawler*)(This - 0x100);
	spEnemyCrawler->m_isDark = false;
	in_rEditParam.CreateParamBool(&spEnemyCrawler->m_isDark, "IsDark");
}

void __declspec(naked) EnemyCrawler_SetModel()
{
	static uint32_t returnAddress = 0xB97C3F;
	static char const* enm_crawler_HD = "enm_crawler_HD";
	static char const* enm_drawler_HD = "enm_drawler_HD";
	__asm
	{
		cmp     byte ptr [esi + 270h], 0
		jz		original
		push	enm_drawler_HD
		jmp		[returnAddress]

		original:
		push	enm_crawler_HD
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyCrawler_SetBody()
{
	static uint32_t returnAddress = 0xB97C6D;
	static char const* enm_crawler_HD_body = "enm_crawler_HD_body";
	static char const* enm_drawler_HD_body = "enm_drawler_HD_body";
	__asm
	{
		cmp     byte ptr [esi + 270h], 0
		jz		original
		push	enm_drawler_HD_body
		jmp		[returnAddress]

		original:
		push	enm_crawler_HD_body
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyCrawler_SetMgm()
{
	static uint32_t returnAddress = 0xB97C91;
	static char const* enm_crawler_HD_mgm = "enm_crawler_HD_mgm";
	static char const* enm_drawler_HD_mgm = "enm_drawler_HD_mgm";
	__asm
	{
		cmp     byte ptr [esi + 270h], 0
		jz		original
		push	enm_drawler_HD_mgm
		jmp		[returnAddress]

		original:
		push	enm_crawler_HD_mgm
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyCrawler_SetAnim()
{
	static uint32_t returnAddress = 0xB97CB5;
	static char const* enm_crawler_HD_mgm_0000 = "enm_crawler_HD_mgm-0000";
	static char const* enm_drawler_HD_mgm_0000 = "enm_drawler_HD_mgm-0000";
	__asm
	{
		cmp     byte ptr [esi + 270h], 0
		jz		original
		push	enm_drawler_HD_mgm_0000
		jmp		[returnAddress]

		original:
		push	enm_crawler_HD_mgm_0000
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyCrawler_SetProjectileData()
{
	static uint32_t returnAddress = 0xB98578;
	static char const* enm_togeball = "enm_togeball";
	static char const* enm_dogeball = "enm_dogeball";
	__asm
	{
		cmp     byte ptr [ebx + 270h], 0
		jz		original
		push	enm_dogeball
		jmp		[returnAddress]

		original:
		push	enm_togeball
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyCrawler_SetExplosionData()
{
	static uint32_t returnAddress = 0xB9860D;
	__asm
	{
		cmp     byte ptr [ebx + 270h], 0
		jz		original
		lea		eax, m_darkTogeballExplosionData
		mov		dword ptr [esp + 0xF8], eax
		jmp		[returnAddress]

		original:
		mov		dword ptr [esp + 0xF8], 0x1E67324
		jmp		[returnAddress]
	}
}

void EnemyCrawler::applyPatches()
{
	// increase alloc size
	WRITE_MEMORY(0xB999F9, uint32_t, c_size + 4); // 2D
	WRITE_MEMORY(0xB99836, uint32_t, c_size + 4); // 3D

	// change model
	INSTALL_HOOK(EnemyCrawler_InitializeEditParam);
	WRITE_JUMP(0xB97C3A, EnemyCrawler_SetModel);
	WRITE_JUMP(0xB97C68, EnemyCrawler_SetBody);
	WRITE_JUMP(0xB97C8C, EnemyCrawler_SetMgm);
	WRITE_JUMP(0xB97CB0, EnemyCrawler_SetAnim);
	WRITE_MEMORY(0x16F95CC + 0xC8, void*, AddCallback);

	// TODO: set fireball data
	WRITE_JUMP(0xB98573, EnemyCrawler_SetProjectileData);
	WRITE_JUMP(0xB98602, EnemyCrawler_SetExplosionData);
}

void __fastcall EnemyCrawler::AddCallback
(
	EnemyCrawler* This, void*, void*
)
{
	uint32_t pAimTargetParams = *(uint32_t*)((uint32_t)This + 0x11C);
	if (This->m_isDark)
	{
		This->m_energyAmount = 3;

		// set IsCrisisCityEnemy to 2
		*(int*)(pAimTargetParams + 0x28) = 2;
	}
	else
	{
		This->m_energyAmount = 2;

		// set IsCrisisCityEnemy to 1
		*(int*)(pAimTargetParams + 0x28) = 1;
	}

	// can only construct in runtime?
	if (!m_darkTogeballExplosionData.m_init)
	{
		m_darkTogeballExplosionData.m_effectName = "ef_en_drk_yh2_impact";
		m_darkTogeballExplosionData.m_init = true;
	}
}
