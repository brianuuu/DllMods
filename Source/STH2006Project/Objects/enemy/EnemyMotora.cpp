#include "EnemyMotora.h"

HOOK(void, __fastcall, EnemyMotora_InitializeEditParam, 0xBC5020, uint32_t This, void* Edx, Sonic::CEditParam& in_rEditParam)
{
	originalEnemyMotora_InitializeEditParam(This, Edx, in_rEditParam);

	EnemyMotora* spEnemyMotora = (EnemyMotora*)(This - 0x100);
	spEnemyMotora->m_isChaser = false;
	in_rEditParam.CreateParamBool(&spEnemyMotora->m_isChaser, "IsChaser");
}

HOOK(int*, __fastcall, EnemyMotora_SpawnBrk, 0xBC3D60, EnemyMotora* This)
{
	int* result = originalEnemyMotora_SpawnBrk(This);

	static Hedgehog::Base::CSharedString enm_motora_brk = "enm_motora_brk";
	static Hedgehog::Base::CSharedString enm_cotora_brk = "enm_cotora_brk";
	static Hedgehog::Base::CSharedString enm_motora_brk_ = "enm_motora_brk_";
	static Hedgehog::Base::CSharedString enm_cotora_brk_ = "enm_cotora_brk_";
	if (This->m_isChaser)
	{
		WRITE_MEMORY(0x1E77874, char*, enm_cotora_brk.get());
		WRITE_MEMORY(0x1E77878, char*, enm_cotora_brk_.get());
	}
	else
	{
		WRITE_MEMORY(0x1E77874, char*, enm_motora_brk.get());
		WRITE_MEMORY(0x1E77878, char*, enm_motora_brk_.get());
	}

	return result;
}

void __declspec(naked) EnemyMotora_SetModel()
{
	static uint32_t returnAddress = 0xBC5FF5;
	static char const* enm_motora_HD = "enm_motora_HD";
	static char const* enm_cotora_HD = "enm_cotora_HD";
	__asm
	{
		cmp     byte ptr [ebx + 260h], 0
		jz		original
		push	enm_cotora_HD
		jmp		[returnAddress]

		original:
		push	enm_motora_HD
		jmp		[returnAddress]
	}
}

void EnemyMotora::applyPatches()
{
	// increase alloc size
	WRITE_MEMORY(0xBC71D6, uint32_t, c_size + 4); // 2D
	WRITE_MEMORY(0xBC7016, uint32_t, c_size + 4); // 3D

	// change model
	INSTALL_HOOK(EnemyMotora_InitializeEditParam);
	INSTALL_HOOK(EnemyMotora_SpawnBrk);
	WRITE_JUMP(0xBC5FF0, EnemyMotora_SetModel);
	WRITE_MEMORY(0x16F67E0, void*, AddCallback);
}

void EnemyMotora::AddCallback
(
	EnemyMotora* This, void*,
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder,
	Sonic::CGameDocument* in_pGameDocument,
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	FUNCTION_PTR(void, __thiscall, fpCEnemyBaseAddCallback, 0xBDF720,
		void* This,
		const Hedgehog::Base::THolder<Sonic::CWorld>&in_rWorldHolder,
		Sonic::CGameDocument * in_pGameDocument,
		const boost::shared_ptr<Hedgehog::Database::CDatabase>&in_spDatabase
	);

	fpCEnemyBaseAddCallback(This, in_rWorldHolder, in_pGameDocument, in_spDatabase);
	if (This->m_isChaser)
	{
		// modify chaos energy amount
		This->m_energyAmount = 2;
	}
}
