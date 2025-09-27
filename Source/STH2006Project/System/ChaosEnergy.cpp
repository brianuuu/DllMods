#include "ChaosEnergy.h"
#include "Configuration.h"

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

HOOK(void, __stdcall, ChaosEnergy_PhysicsReward, 0xEA49B0, uint32_t This, int a2, void* a3, bool a4)
{
	// Check if it's a breakable object
	if (!*(bool*)(This + 0x120))
	{
		if (!a4 || *(uint32_t*)(This + 0x108) == 1)
		{
			std::string name(*(char**)(This + 0x130));
			int fakeEnemyType = ChaosEnergy::getFakeEnemyType(name);
			if (fakeEnemyType)
			{
				float* pos = (float*)((*(uint32_t*)(This + 0xB8)) + 0x70);
				Common::SpawnBoostParticle((uint32_t**)This, hh::math::CVector(pos[0], pos[1], pos[2]), fakeEnemyType);
			}
		}
	}

	originalChaosEnergy_PhysicsReward(This, a2, a3, a4);
}

HOOK(bool, __fastcall, ChaosEnergy_CObjectPhysicsProcessMessage, 0xEA5370, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
	if (flag && message.Is<Sonic::Message::MsgGetEnemyType>())
	{
		auto& msg = static_cast<Sonic::Message::MsgGetEnemyType&>(message);

		uint32_t pObjectBase = (uint32_t)static_cast<Sonic::CObjectBase*>(This);
		std::string name(*(char**)(pObjectBase + 0x130));
		int fakeEnemyType = ChaosEnergy::getFakeEnemyType(name);
		if (fakeEnemyType)
		{
			*msg.m_pType = 2;
		}

		return true;
	}

	return originalChaosEnergy_CObjectPhysicsProcessMessage(This, Edx, message, flag);
}

void __declspec(naked) setChaosEnergySfxPfx()
{
	static uint32_t returnAddress = 0x112459F;
	__asm
	{
		// Change sound effect
		mov		eax, [esp + 8h] // This is pushed in the stack
		cmp		dword ptr [eax + 11Ch], 0
		push	ebx
		push	ecx
		je		jump

		// LightCore
		mov		ecx, 1
		call	ChaosEnergy::playChaosEnergyPfx
		pop		ecx
		pop		ebx
		mov		eax, 4002086
		jmp		[returnAddress]

		// Chaos Drive
		jump:
		mov		ecx, 0
		call	ChaosEnergy::playChaosEnergyPfx
		pop		ecx
		pop		ebx
		mov		eax, 4002087 
		jmp		[returnAddress]
	}
}

void __declspec(naked) getEnemyChaosEnergyType()
{
	static uint32_t returnAddress = 0xBE05F7;
	__asm
	{
		mov		edx, ecx
		mov		ecx, esi
		call	ChaosEnergy::getEnemyChaosEnergyTypeImpl
		mov		ecx, eax

		// original function
		mov     edx, [esi]
		mov     eax, [edx + 9Ch]
		jmp		[returnAddress]
	}
}

void __declspec(naked) setChaosEnergyAmountAndType()
{
	static uint32_t returnAddress = 0x112509D;
	__asm
	{
		// Set amount
		xor		edx, edx
		mov		dx, word ptr [edi + 10h]
		mov		[esi + 110h], edx

		// Set type
		mov		dx, word ptr [edi + 12h]
		mov		[esi + 11Ch], edx

		jmp		[returnAddress]
	}
}

const char* volatile const lightcoreEffectName = "ef_if_hud_yh1_lightcore";
void __declspec(naked) swapChaosEnergyEffect()
{
	static uint32_t returnAddress = 0x1124367;
	__asm
	{
		mov		ecx, [esi]
		mov		ecx, [ecx + 11Ch]
		test	cl, cl
		jz		jump

		push	lightcoreEffectName
		jmp		[returnAddress]

		jump:
		push	[0x1613B98] // ef_if_hud_yh1_boostenergy
		jmp		[returnAddress]
	}
}

void ChaosEnergy::applyPatches()
{
	// Make Chaos Energy goes to Sonic
	INSTALL_HOOK(ChaosEnergy_MsgGetHudPosition);

	// Change enemy spawning ChaosDrive/LightCore
	WRITE_JUMP(0xBE05EF, getEnemyChaosEnergyType);

	// Set amount of ChaosDrive/LightCore to spawn from enemy
	WRITE_JUMP(0x1125094, setChaosEnergyAmountAndType);

	// Swap between ChaosDrive and LightCore particle effect
	WRITE_JUMP(0x1124362, swapChaosEnergyEffect);

	// Use correct ChaosDrive/LightCore sfx and pfx
	WRITE_JUMP(0x112459A, setChaosEnergySfxPfx);

	// Spawn chaos energy base on currect trick level (visual only)
	WRITE_MEMORY(0x16D1970, uint32_t, 1, 1, 2, 3);

	// Give 3 chaos energy for board trick jump (visual only)
	WRITE_MEMORY(0x11A12E4, uint8_t, 3);

	// Set absorb time to 1.2s
	static float ChaosEnergyParam[] =
	{
		5.0f,	// UpHeight
		0.3f,	// UpTime
		1.2f	// AbsorbTime
	};
	WRITE_MEMORY(0xC8EF3D, float*, ChaosEnergyParam);
	WRITE_MEMORY(0x11244A6, float*, ChaosEnergyParam);

	// Make all spawn 1 chaos energy but multiply add boost by 5
	WRITE_MEMORY(0xBDF7F2, uint32_t, 1); // enemy
	WRITE_MEMORY(0xC8E0D0, uint32_t, 1); // CObjEnergyMeteor

	static float const ChaosEnergyRecoveryRate1 = 0.15f; // Enemy (0.03 default)
	WRITE_MEMORY(0xD96724, float*, &ChaosEnergyRecoveryRate1);

	static float const ChaosEnergyRecoveryRate3 = 0.1f; // EnemyBonus (0.02 default)
	WRITE_MEMORY(0xD96736, float*, &ChaosEnergyRecoveryRate3);
	WRITE_MEMORY(0xE60C94, float*, &ChaosEnergyRecoveryRate3);

	// Spawn Chaos Energy on fake enemy object physics
	INSTALL_HOOK(ChaosEnergy_PhysicsReward);
	INSTALL_HOOK(ChaosEnergy_CObjectPhysicsProcessMessage);
}

uint32_t __fastcall ChaosEnergy::getEnemyChaosEnergyTypeImpl(uint32_t pCEnemyBase, uint32_t amount)
{
	uint32_t const pAimTargetParams = *(uint32_t*)(pCEnemyBase + 0x11C);
	bool const isCrisisCityEnemy = *(bool*)(pAimTargetParams + 0x28);

	// For Iblis monsters, add 0x00010000 to notify it to use lightcore
	if (isCrisisCityEnemy)
	{
		amount += 0x00010000;
	}

	return amount;
}

void __fastcall ChaosEnergy::playChaosEnergyPfx(bool isLightcore)
{
	static SharedPtrTypeless pfxHandle;
	void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
	Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle, matrixNode, isLightcore ? "ef_if_hud_yh1_lightcore_get" : "ef_if_hud_yh1_boostenergy_get" , 1);
}

int ChaosEnergy::getFakeEnemyType(std::string const& name)
{
	// Return if provided object physics name is one of the fake enemies

	static std::vector<std::string> const fakeBigEnemyList =
	{
		"en_eBuster",
	};

	static std::vector<std::string> const fakeMediumEnemyList =
	{
		"en_eLancer",
		"en_eArmor",
		"en_eSweeper", // stealth
		"en_eChaser",
		"en_eCommander",
	};

	// Special case for Egg Chaser exploding
	if (name.find("enm_eggchaser") != std::string::npos)
	{
		return 1;
	}

	// all fake enemy must have "en_e" and "_col" in their name
	if (name.find("en_e") != std::string::npos && name.find("_col") != std::string::npos)
	{
		// Type big enemies
		for (std::string const& enemy : fakeBigEnemyList)
		{
			if (name.find(enemy) != std::string::npos)
			{
				return 3;
			}
		}

		// Type medium enemies
		for (std::string const& enemy : fakeMediumEnemyList)
		{
			if (name.find(enemy) != std::string::npos)
			{
				return 2;
			}
		}
	
		// Other fake enemies, all type small
		return 1;
	}

	return 0;
}
