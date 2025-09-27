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

	static uint32_t __fastcall getEnemyChaosEnergyTypeImpl(uint32_t pCEnemyBase, uint32_t amount);

	static void __fastcall playChaosEnergyPfx(bool isLightcore);

	static int getFakeEnemyType(std::string const& name);
};

