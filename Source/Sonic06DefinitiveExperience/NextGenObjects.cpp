#include "NextGenObjects.h"

void NextGenObjects::applyPatches()
{
    // Checkpoint
    WRITE_NOP(0x1032305, 7); // SV pole point 90 degree like FV
    WRITE_JUMP(0x10323F8, (void*)0x1032646); // Disable middle laser effect
    WRITE_MEMORY(0x1032879, uint32_t, 0x1693ECC); // mat restart -> 1.0f

    // TailsDashRing
    WRITE_MEMORY(0x1A6B800, uint32_t, 4001020); // sfx to DashRing
}
