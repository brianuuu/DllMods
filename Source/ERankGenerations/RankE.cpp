#include "RankE.h"

void RankE::applyPatches()
{
	WRITE_MEMORY(0x15E8DB6, uint8_t, 0x35); // sn_result00_loop -> sn_result04_loop
	WRITE_MEMORY(0x15E8DCA, uint8_t, 0x35); // sn_result00 -> sn_result04
	WRITE_MEMORY(0x15D5FFB, uint8_t, 0x35); // ssn_result00_loop -> ssn_result04_loop
	WRITE_MEMORY(0x15D600F, uint8_t, 0x35); // ssn_result00 -> ssn_result04

	WRITE_MEMORY(0x15EFE9D, uint8_t, 0x45); // SonicRankS -> SonicRankE
}
