#include "EnemyGanigani.h"

HOOK(void, __fastcall, EnemyGanigani_InitializeEditParam, 0xBC8C40, uint32_t This, void* Edx, Sonic::CEditParam& in_rEditParam)
{
	originalEnemyGanigani_InitializeEditParam(This, Edx, in_rEditParam);

	EnemyGanigani* spEnemyGanigani = (EnemyGanigani*)(This - 0x100);
	spEnemyGanigani->m_isSweeper = false;
	spEnemyGanigani->m_isArmor = false;
	in_rEditParam.CreateParamBool(&spEnemyGanigani->m_isSweeper, "IsSweeper");
	in_rEditParam.CreateParamBool(&spEnemyGanigani->m_isArmor, "IsArmor");
}

HOOK(int*, __fastcall, EnemyGanigani_SpawnBrk, 0xBC75D0, EnemyGanigani* This)
{
	int* result = originalEnemyGanigani_SpawnBrk(This);

	static Hedgehog::Base::CSharedString enm_ganigani_brk = "enm_ganigani_brk";
	static Hedgehog::Base::CSharedString enm_sanigani_brk = "enm_sanigani_brk";
	static Hedgehog::Base::CSharedString enm_aanigani_brk = "enm_aanigani_brk";
	static Hedgehog::Base::CSharedString enm_ganigani_brk_ = "enm_ganigani_brk_";
	static Hedgehog::Base::CSharedString enm_sanigani_brk_ = "enm_sanigani_brk_";
	static Hedgehog::Base::CSharedString enm_aanigani_brk_ = "enm_aanigani_brk_";
	if (This->m_isSweeper)
	{
		WRITE_MEMORY(0x1E77884, char*, enm_sanigani_brk.get());
		WRITE_MEMORY(0x1E77888, char*, enm_sanigani_brk_.get());
	}
	else if (This->m_isArmor)
	{
		WRITE_MEMORY(0x1E77884, char*, enm_aanigani_brk.get());
		WRITE_MEMORY(0x1E77888, char*, enm_aanigani_brk_.get());
	}
	else
	{
		WRITE_MEMORY(0x1E77884, char*, enm_ganigani_brk.get());
		WRITE_MEMORY(0x1E77888, char*, enm_ganigani_brk_.get());
	}

	return result;
}

HOOK(void, __fastcall, EnemyGanigani_MsgGetHomingAttackPosition, 0xBC7620, EnemyGanigani* This, void* Edx, Sonic::Message::MsgGetHomingAttackPosition& message)
{
	*message.m_pPosition = This->m_spMatrixNodeTransform->m_Transform.m_Position + This->m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitY() * 0.4f;
}

void __declspec(naked) EnemyGanigani_SetModel()
{
	static uint32_t returnAddress = 0xBC9E5A;
	static char const* enm_ganigani_HD = "enm_ganigani_HD";
	static char const* enm_sanigani_HD = "enm_sanigani_HD";
	static char const* enm_aanigani_HD = "enm_aanigani_HD";
	__asm
	{
		cmp     byte ptr [ebx + 250h], 0
		jz		armor
		push	enm_sanigani_HD
		jmp		[returnAddress]

		armor:
		cmp     byte ptr [ebx + 251h], 0
		jz		original
		push	enm_aanigani_HD
		jmp		[returnAddress]

		original:
		push	enm_ganigani_HD
		jmp		[returnAddress]
	}
}

void __declspec(naked) EnemyGanigani_SetCollision()
{
	static uint32_t returnAddress = 0xBCAEB0;
	__asm
	{
		mov     edi, [esp + 30h]
		mov		eax, 0x1E0AF24
		mov		eax, [eax]
		push	eax
		jmp		[returnAddress]
	}
}

void EnemyGanigani::applyPatches()
{
	// increase alloc size
	WRITE_MEMORY(0xBCB756, uint32_t, c_size + 4); // 2D
	WRITE_MEMORY(0xBCB596, uint32_t, c_size + 4); // 3D

	// change model
	INSTALL_HOOK(EnemyGanigani_InitializeEditParam);
	INSTALL_HOOK(EnemyGanigani_SpawnBrk);
	INSTALL_HOOK(EnemyGanigani_MsgGetHomingAttackPosition);
	WRITE_JUMP(0xBC9E55, EnemyGanigani_SetModel);
	WRITE_MEMORY(0x16F62B4 + 0xC8, void*, AddCallback);

	// remove interaction type, change collision size
	WRITE_JUMP(0xBCAEAB, EnemyGanigani_SetCollision);
	WRITE_MEMORY(0xBCAD07, uint32_t, 0x149CCD8); // height -> 0.4
	WRITE_MEMORY(0xBCAF1D, uint32_t, 0x149CCD8); // height -> 0.4
	WRITE_MEMORY(0xBCAF25, uint32_t, 0x13FF820); // xSize -> 0.8

	// don't drop with gravity
	WRITE_JUMP(0x5F6343, (void*)0x5F6383);
}

void EnemyGanigani::AddCallback
(
	EnemyGanigani* This, void*, void*
)
{
	// modify chaos energy amount
	if (This->m_isSweeper)
	{
		This->m_energyAmount = 3;
	}
	else if (This->m_isArmor)
	{
		This->m_energyAmount = 2;
	}
}
