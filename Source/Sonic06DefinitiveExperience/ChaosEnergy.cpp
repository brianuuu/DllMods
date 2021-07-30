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
			sonicPosition.y() += 0.5f; // half Sonic's height
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
		cmp		[esi], 0
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

void ChaosEnergy::applyPatches()
{
	// Make Chaos Energy goes to Sonic
	INSTALL_HOOK(ChaosEnergy_MsgGetHudPosition);

	if (Configuration::m_physics)
	{
		// Don't boost rewards, handle them ourselves
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
		// TODO: bigger enemies reward 2 instead of 1
		WRITE_MEMORY(0xBDF7F2, uint32_t, 1);
	}
}
