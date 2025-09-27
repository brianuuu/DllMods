#include "EnemyBeeton.h"

void EnemyBeeton::applyPatches()
{
	// fix chaos energy amount
	WRITE_MEMORY(0x16F5198, void*, AddCallback);
}

void EnemyBeeton::AddCallback
(
	EnemyBeeton* This, void*,
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
	if (!This->m_isSearcher)
	{
		// modify chaos energy amount
		This->m_energyAmount = 2;
	}
}
