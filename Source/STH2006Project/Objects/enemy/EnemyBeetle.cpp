#include "EnemyBeetle.h"

void EnemyBeetle::applyPatches()
{
	// change model (GunBeetle only)
	WRITE_STRING(0x156BF4C, "enm_feetle_brk");
	WRITE_STRING(0x156BF3C, "enm_feetle_brk_");

	// fix chaos energy amount (MonoBeetle only)
	WRITE_MEMORY(0x16F87CC + 0xC8, void*, AddCallback);

	// CEnemyBeetle size
	WRITE_MEMORY(0xBA59BF, uint32_t, 0x1574644); // damage radius 0.9 -> 1.0
	WRITE_MEMORY(0xBA5A86, uint32_t, 0x14B3D90); // lock radius 0.6 -> 0.9
}

bool EnemyBeetle::isBuster() const
{
	// check if vtable is CEnemyMonoBeetle
	return *(uint32_t*)this == 0x16F87CC;
}

void EnemyBeetle::AddCallback
(
	EnemyBeetle* This, void*, void*
)
{
	// modify chaos energy amount
	This->m_energyAmount = 3;
}
