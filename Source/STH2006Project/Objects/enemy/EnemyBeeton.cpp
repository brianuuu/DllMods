#include "EnemyBeeton.h"

void EnemyBeeton::applyPatches()
{
	// fix chaos energy amount
	WRITE_MEMORY(0x16F517C + 0xC8, void*, AddCallback);
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
