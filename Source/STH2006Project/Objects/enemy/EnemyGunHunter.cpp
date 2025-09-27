#include "EnemyGunHunter.h"

void EnemyGunHunter::applyPatches()
{
    // Gunner ignore slip damage
    WRITE_MEMORY(0xBAA40F, uint8_t, 0xEB);
}
