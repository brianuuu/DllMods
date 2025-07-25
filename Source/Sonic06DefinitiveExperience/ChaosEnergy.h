/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Changes how Chaos Energy awards boost
/*----------------------------------------------------------*/

#pragma once
class ChaosEnergy
{
public:
	static void applyPatches();

	static uint32_t __fastcall getEnemyChaosEnergyAmountImpl(uint32_t* pEnemy);
	static uint32_t __fastcall getEnemyChaosEnergyTypeImpl(uint32_t* pEnemy, uint32_t amount);

	static void __fastcall playChaosEnergyPfx(bool isLightcore);
	static void setChaosEnergyRewardOverride(float amount = 0.0f);
};

