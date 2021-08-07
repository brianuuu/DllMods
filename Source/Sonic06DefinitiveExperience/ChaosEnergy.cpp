#include "ChaosEnergy.h"
#include "Configuration.h"

static float const c_chaosEnergyReward = 5.0f;

HOOK(void, __fastcall, ChaosEnergy_MsgGetHudPosition, 0x1096790, void* This, void* Edx, MsgGetHudPosition* message)
{
	if (message->m_type == 0)
	{
		Eigen::Vector3f sonicPosition;
		Eigen::Quaternionf sonicRotation;
		if (Common::GetPlayerTransform(sonicPosition, sonicRotation))
		{
			sonicPosition += sonicRotation * (Eigen::Vector3f::UnitY() * 0.5f); // half Sonic's height
			message->m_position = sonicPosition;
			return;
		}
	}

	originalChaosEnergy_MsgGetHudPosition(This, Edx, message);
}

// Play rainbow ring voice
uint32_t fpAddBoostToSonicContext = 0xE5D990;
uint32_t addBoostFromChaosEnergyReturnAddress = 0x112459F;
void __declspec(naked) addBoostFromChaosEnergy()
{
	__asm
	{
		// Check Sonic context just in case
		mov		esi, PLAYER_CONTEXT
		cmp		dword ptr [esi], 0
		je		jump

		// Award Sonic 5 boost
		push	ecx
		push	edi
		push	0
		movss	xmm1, c_chaosEnergyReward
		mov		esi, [esi]
		call	[fpAddBoostToSonicContext]
		pop		edi
		pop		ecx

		// original function
		jump:
		mov		eax, 4002073 // cue ID
		jmp		[addBoostFromChaosEnergyReturnAddress]
	}
}

uint32_t getEnemyChaosEnergyAmountReturnAddress = 0xBE05EF;
void __declspec(naked) getEnemyChaosEnergyAmount()
{
	__asm
	{
		mov		ecx, esi
		call	ChaosEnergy::getEnemyChaosEnergyAmountImpl
		mov		ecx, eax
		jmp		[getEnemyChaosEnergyAmountReturnAddress]
	}
}

void ChaosEnergy::applyPatches()
{
	// Make Chaos Energy goes to Sonic
	INSTALL_HOOK(ChaosEnergy_MsgGetHudPosition);

	if (Configuration::m_physics)
	{
		// Don't give boost rewards, handle them ourselves
		WRITE_JUMP(0xE1827B, (void*)0xE182E0); // MsgDamageSuccess
		WRITE_MEMORY(0x11A128F, uint8_t, 0x83, 0xC4, 0x04, 0x90, 0x90); // Board trick jump

		// Don't reward boost from enemy spawned chaos energy
		WRITE_JUMP(0xE60C6C, (void*)0xE60D79);

		// Award 5 boost when chaos energy reach Sonic
		WRITE_JUMP(0x112459A, addBoostFromChaosEnergy);

		// Spawn chaos energy base on currect trick level
		WRITE_MEMORY(0x16D1970, uint32_t, 1, 1, 2, 3);

		// Give 3 chaos energy for board trick jump
		WRITE_MEMORY(0x11A12E4, uint8_t, 3);

		// Change number of chaos energy spawn from enemies
		WRITE_JUMP(0xBE05E9, getEnemyChaosEnergyAmount);
	}
}

uint32_t __fastcall ChaosEnergy::getEnemyChaosEnergyAmountImpl(uint32_t* pEnemy)
{
	//printf("0x%08X\n", pEnemy[0]);
	switch (pEnemy[0])
	{
	case 0x016F7C9C: // CEnemyEggRobo
	{
		// CEnemyEggRobo[104] == 1 -> missile
		return pEnemy[104] ? 1 : 2;
	}
	case 0x016FB1FC: return 3; // CEnemyELauncher
	case 0x016F95CC: return 2; // CEnemyCrawler
	default: return 1;
	}
}
