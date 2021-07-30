#include "ChaosEnergy.h"

static float const c_chaosEnergyRecoveryRate = 0.05f;

HOOK(void, __fastcall, ChaosEnergy_GetHudPosition, 0x1096790, void* This, void* Edx, MsgGetHudPosition* message)
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

	originalChaosEnergy_GetHudPosition(This, Edx, message);
}

// Play rainbow ring voice
uint32_t forceChaosEnergyRateReturnAddress = 0xE60D29;
void __declspec(naked) forceChaosEnergyRate()
{
	__asm
	{
		// original function
		ja		jump

		// override rate
		movss	xmm1, c_chaosEnergyRecoveryRate

		jump:
		jmp		[forceChaosEnergyRateReturnAddress]
	}
}

void ChaosEnergy::applyPatches()
{
	// Make Chaos Energy goes to Sonic
	INSTALL_HOOK(ChaosEnergy_GetHudPosition);

	// Chaos energy always award 5% boost (20 to max)
	WRITE_JUMP(0xE60D24, forceChaosEnergyRate);

	// Don't reward chaos energy on object physics
	WRITE_NOP(0xE18280, 0x7);

	// Change number of chaos energy spawn from enemies
	// TODO: bigger enemies reward 2 instead of 1
	WRITE_MEMORY(0xBDF7F2, uint32_t, 1);
}
