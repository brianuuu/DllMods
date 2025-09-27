/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by eGunner
/*----------------------------------------------------------*/

#pragma once
class EnemyGunHunter : public Sonic::CObjectBase
{
public:
	static uint32_t const c_size = 0x3D0;
	INSERT_PADDING(0x18);
	uint32_t m_energyAmount = 1;
	INSERT_PADDING(0x2B4);

public:
	static void applyPatches();
};

BB_ASSERT_OFFSETOF(EnemyGunHunter, m_energyAmount, 0x118);
BB_ASSERT_SIZEOF(EnemyGunHunter, EnemyGunHunter::c_size);
