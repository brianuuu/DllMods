#include "EnemyBeeton.h"

void EnemyBeeton::applyPatches()
{
	// fix chaos energy amount
	WRITE_MEMORY(0x16F517C + 0xC8, void*, AddCallback);

	// CEnemyBeeton size
	WRITE_MEMORY(0xBDB1FE, uint32_t, 0x1574644); // body radius -> 1.0
}

void EnemyBeeton::AddCallback
(
	EnemyBeeton* This, void*, void*
)
{
	// modify chaos energy amount
	if (!This->m_isSearcher)
	{
		This->m_energyAmount = 2;
	}
}
